
/*
 * eval.c — Tree-walking interpreter for C99 ASTs.
 *
 * Walks the AST produced by the parser and executes it directly.
 * Supports: integers, floats, pointers, arrays, structs, strings, functions,
 *           all statement types, global/local variables, recursion, printf.
 */

/* Helper: compare a String (not null-terminated) against a C string literal */
internal b32 str_eq(String s, const char *lit) {
    s32 len = (s32)strlen(lit);
    return ((s32)s.count == len) && (memcmp(s.data, lit, len) == 0);
}

/* Helper: compare two Strings (not null-terminated) for equality */
internal b32 str_eq2(String a, String b) {
    return (a.count == b.count) && (a.count == 0 || memcmp(a.data, b.data, a.count) == 0);
}

/* ======================================================================
 * Value constructors
 * ====================================================================== */

internal Val val_void(void) {
    Val v; v.kind = VAL_VOID; v.i = 0; return v;
}

internal Val val_int(s64 v) {
    Val r; r.kind = VAL_INT; r.i = v; return r;
}

internal Val val_float(f64 v) {
    Val r; r.kind = VAL_FLOAT; r.f = v; return r;
}

internal Val val_ptr(Val *p) {
    Val r; r.kind = VAL_PTR; r.ptr = p; return r;
}

internal Val val_string(char *s) {
    Val r; r.kind = VAL_STRING; r.str = s; return r;
}

internal Val val_func(ASTDecl *decl) {
    Val r; r.kind = VAL_FUNC; r.func = decl; return r;
}

internal Val val_array(s32 len) {
    Val r;
    r.kind = VAL_ARRAY;
    r.arr.len = len;
    r.arr.elems = (Val *)calloc(len, sizeof(Val));
    return r;
}

internal Val val_struct(String name, s32 num_fields) {
    Val r;
    r.kind = VAL_STRUCT;
    r.strct.name = name;
    r.strct.num_fields = num_fields;
    r.strct.fields = (StructField *)calloc(num_fields, sizeof(StructField));
    return r;
}

/* ======================================================================
 * Value conversion helpers
 * ====================================================================== */

internal s64 val_to_int(Val v) {
    switch (v.kind) {
        case VAL_INT:    return v.i;
        case VAL_FLOAT:  return (s64)v.f;
        case VAL_PTR:    return (v.ptr != NULL) ? 1 : 0;
        case VAL_STRING: return (v.str != NULL) ? 1 : 0;
        default:         return 0;
    }
}

internal f64 val_to_float(Val v) {
    switch (v.kind) {
        case VAL_FLOAT:  return v.f;
        case VAL_INT:    return (f64)v.i;
        default:         return 0.0;
    }
}

internal b32 val_is_truthy(Val v) {
    switch (v.kind) {
        case VAL_INT:    return v.i != 0;
        case VAL_FLOAT:  return v.f != 0.0;
        case VAL_PTR:    return v.ptr != NULL;
        case VAL_STRING: return v.str != NULL;
        case VAL_FUNC:   return v.func != NULL;
        case VAL_VOID:   return 0;
        default:         return 0;
    }
}

/* Deep copy a Val (for assignment semantics) */
internal Val val_copy(Val src) {
    Val dst = src;
    if (src.kind == VAL_ARRAY) {
        dst.arr.elems = (Val *)malloc(src.arr.len * sizeof(Val));
        for (s32 i = 0; i < src.arr.len; ++i)
            dst.arr.elems[i] = val_copy(src.arr.elems[i]);
    } else if (src.kind == VAL_STRUCT) {
        dst.strct.fields = (StructField *)malloc(src.strct.num_fields * sizeof(StructField));
        for (s32 i = 0; i < src.strct.num_fields; ++i) {
            dst.strct.fields[i].name = src.strct.fields[i].name;
            dst.strct.fields[i].value = (Val *)malloc(sizeof(Val));
            *dst.strct.fields[i].value = val_copy(*src.strct.fields[i].value);
        }
    } else if (src.kind == VAL_STRING && src.str) {
        s32 len = (s32)strlen(src.str);
        dst.str = (char *)malloc(len + 1);
        memcpy(dst.str, src.str, len + 1);
    }
    return dst;
}

/* ======================================================================
 * sizeof for types
 * ====================================================================== */

internal s64 type_sizeof(TypeSpec *type) {
    if (!type) return 0;
    switch (type->kind) {
        case TYPESPEC_NAME: {
            String n = type->name.name;
            if (!n.data) return 0;
            if (str_eq(n, "char"))   return 1;
            if (str_eq(n, "short"))  return 2;
            if (str_eq(n, "int"))    return 4;
            if (str_eq(n, "long"))   return 8;
            if (str_eq(n, "float"))  return 4;
            if (str_eq(n, "double")) return 8;
            if (str_eq(n, "void"))   return 0;
            return 8;
        }
        case TYPESPEC_PTR: return 8;
        case TYPESPEC_ARRAY: {
            s64 elem_sz = type_sizeof(type->array.elem);
            /* size can be NULL for unsized arrays */
            return elem_sz * 8;
        }
        case TYPESPEC_CONST:
        case TYPESPEC_VOLATILE:
            return type_sizeof(type->ptr.base);
        default: return 8;
    }
}

/* ======================================================================
 * Runtime error reporting
 * ====================================================================== */

internal void interp_error(Interp *interp, u32 line, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "runtime error (line %u): ", line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    interp->had_error = 1;
}

/* ======================================================================
 * Scope / Environment management
 * ====================================================================== */

internal Scope *scope_new(Scope *parent) {
    Scope *s = (Scope *)calloc(1, sizeof(Scope));
    s->parent = parent;
    s->bindings = NULL;
    return s;
}

internal Scope *scope_push(Interp *interp) {
    Scope *s = scope_new(interp->current_scope);
    interp->current_scope = s;
    return s;
}

internal void scope_pop(Interp *interp) {
    Scope *old = interp->current_scope;
    interp->current_scope = old->parent;
    darr_free(old->bindings);
    free(old);
}

internal void scope_define(Scope *scope, String name, Val val) {
    /* If already defined in this scope, overwrite */
    s32 len = (s32)darr_len(scope->bindings);
    for (s32 i = 0; i < len; ++i) {
        if (str_eq2(scope->bindings[i].name, name)) {
            scope->bindings[i].val = val;
            return;
        }
    }
    Binding b;
    b.name = name;
    b.val = val;
    darr_push(scope->bindings, b);
}

internal Val *scope_lookup_local(Scope *scope, String name) {
    s32 len = (s32)darr_len(scope->bindings);
    for (s32 i = 0; i < len; ++i) {
        if (str_eq2(scope->bindings[i].name, name))
            return &scope->bindings[i].val;
    }
    return NULL;
}

internal Val *scope_lookup(Interp *interp, String name) {
    for (Scope *s = interp->current_scope; s; s = s->parent) {
        Val *v = scope_lookup_local(s, name);
        if (v) return v;
    }
    return NULL;
}

/* ======================================================================
 * Builtins
 * ====================================================================== */

/* Process a printf format string using our Val arguments */
internal Val builtin_printf(Interp *interp, Val *args, s32 num_args) {
    (void)interp;
    if (num_args < 1) return val_int(0);

    const char *fmt = NULL;
    if (args[0].kind == VAL_STRING) fmt = args[0].str;
    else if (args[0].kind == VAL_PTR && args[0].ptr && args[0].ptr->kind == VAL_STRING)
        fmt = args[0].ptr->str;
    if (!fmt) { printf("(null)"); return val_int(0); }

    s32 ai = 1;
    s32 chars_printed = 0;
    for (const char *p = fmt; *p; ++p) {
        if (*p == '%') {
            ++p;
            if (!*p) break;

            /* Collect flags/width/precision — simplified */
            while (*p == '-' || *p == '+' || *p == ' ' || *p == '0' || *p == '#') ++p;
            while (*p >= '0' && *p <= '9') ++p;
            if (*p == '.') { ++p; while (*p >= '0' && *p <= '9') ++p; }
            /* length modifiers */
            if (*p == 'l') { ++p; if (*p == 'l') ++p; }
            else if (*p == 'h') { ++p; if (*p == 'h') ++p; }

            switch (*p) {
                case 'd': case 'i': {
                    s64 v = (ai < num_args) ? val_to_int(args[ai++]) : 0;
                    chars_printed += printf("%lld", (long long)v);
                } break;
                case 'u': {
                    u64 v = (u64)((ai < num_args) ? val_to_int(args[ai++]) : 0);
                    chars_printed += printf("%llu", (unsigned long long)v);
                } break;
                case 'x': case 'X': {
                    u64 v = (u64)((ai < num_args) ? val_to_int(args[ai++]) : 0);
                    chars_printed += printf((*p == 'x') ? "%llx" : "%llX", (unsigned long long)v);
                } break;
                case 'o': {
                    u64 v = (u64)((ai < num_args) ? val_to_int(args[ai++]) : 0);
                    chars_printed += printf("%llo", (unsigned long long)v);
                } break;
                case 'f': case 'F': {
                    f64 v = (ai < num_args) ? val_to_float(args[ai++]) : 0.0;
                    chars_printed += printf("%f", v);
                } break;
                case 'e': case 'E': {
                    f64 v = (ai < num_args) ? val_to_float(args[ai++]) : 0.0;
                    chars_printed += printf((*p == 'e') ? "%e" : "%E", v);
                } break;
                case 'g': case 'G': {
                    f64 v = (ai < num_args) ? val_to_float(args[ai++]) : 0.0;
                    chars_printed += printf((*p == 'g') ? "%g" : "%G", v);
                } break;
                case 'c': {
                    s64 v = (ai < num_args) ? val_to_int(args[ai++]) : 0;
                    chars_printed += printf("%c", (char)v);
                } break;
                case 's': {
                    const char *sv = "(null)";
                    if (ai < num_args) {
                        Val a = args[ai++];
                        if (a.kind == VAL_STRING && a.str) sv = a.str;
                        else if (a.kind == VAL_PTR && a.ptr && a.ptr->kind == VAL_STRING && a.ptr->str)
                            sv = a.ptr->str;
                    }
                    chars_printed += printf("%s", sv);
                } break;
                case 'p': {
                    s64 v = (ai < num_args) ? val_to_int(args[ai++]) : 0;
                    chars_printed += printf("%p", (void *)(intptr_t)v);
                } break;
                case '%':
                    putchar('%'); chars_printed++;
                    break;
                case 'n': ai++; break;
                default:
                    putchar('%'); putchar(*p); chars_printed += 2;
                    break;
            }
        } else {
            putchar(*p);
            chars_printed++;
        }
    }
    fflush(stdout);
    return val_int(chars_printed);
}

internal Val builtin_putchar(Interp *interp, Val *args, s32 num_args) {
    (void)interp;
    if (num_args >= 1) {
        s64 c = val_to_int(args[0]);
        putchar((int)c);
        return val_int(c);
    }
    return val_int(-1);
}

internal Val builtin_puts(Interp *interp, Val *args, s32 num_args) {
    (void)interp;
    if (num_args >= 1) {
        const char *s = "(null)";
        if (args[0].kind == VAL_STRING && args[0].str) s = args[0].str;
        return val_int(puts(s));
    }
    return val_int(-1);
}

internal Val builtin_malloc(Interp *interp, Val *args, s32 num_args) {
    (void)interp;
    if (num_args >= 1) {
        s64 size = val_to_int(args[0]);
        Val *block = (Val *)calloc(size > 0 ? size : 1, sizeof(Val));
        return val_ptr(block);
    }
    return val_ptr(NULL);
}

internal Val builtin_calloc(Interp *interp, Val *args, s32 num_args) {
    (void)interp;
    if (num_args >= 2) {
        s64 count = val_to_int(args[0]);
        s64 size = val_to_int(args[1]);
        s64 total = count * size;
        Val *block = (Val *)calloc(total > 0 ? total : 1, sizeof(Val));
        return val_ptr(block);
    }
    return val_ptr(NULL);
}

internal Val builtin_free(Interp *interp, Val *args, s32 num_args) {
    (void)interp; (void)args; (void)num_args;
    /* In our interpreter, we don't truly free individual Vals */
    return val_void();
}

internal Val builtin_exit(Interp *interp, Val *args, s32 num_args) {
    (void)interp;
    s64 code = (num_args >= 1) ? val_to_int(args[0]) : 0;
    exit((int)code);
    return val_void();
}

internal Val builtin_abs(Interp *interp, Val *args, s32 num_args) {
    (void)interp;
    if (num_args >= 1) {
        s64 v = val_to_int(args[0]);
        return val_int(v < 0 ? -v : v);
    }
    return val_int(0);
}

internal Val builtin_strlen(Interp *interp, Val *args, s32 num_args) {
    (void)interp;
    if (num_args >= 1 && args[0].kind == VAL_STRING && args[0].str)
        return val_int((s64)strlen(args[0].str));
    return val_int(0);
}

internal Val builtin_strcmp(Interp *interp, Val *args, s32 num_args) {
    (void)interp;
    if (num_args >= 2) {
        const char *a = (args[0].kind == VAL_STRING) ? args[0].str : "";
        const char *b = (args[1].kind == VAL_STRING) ? args[1].str : "";
        return val_int(strcmp(a ? a : "", b ? b : ""));
    }
    return val_int(0);
}

internal Val builtin_atoi(Interp *interp, Val *args, s32 num_args) {
    (void)interp;
    if (num_args >= 1 && args[0].kind == VAL_STRING && args[0].str)
        return val_int(atoi(args[0].str));
    return val_int(0);
}

internal Val builtin_sprintf(Interp *interp, Val *args, s32 num_args) {
    (void)interp;
    /* Simplified: write into a buffer pointed to by args[0], format in args[1] */
    if (num_args < 2) return val_int(0);
    /* Just pass through to printf for now but capture it */
    /* For simplicity, use a static buffer */
    char buf[4096];
    const char *fmt = NULL;
    if (args[1].kind == VAL_STRING) fmt = args[1].str;
    if (!fmt) return val_int(0);

    s32 ai = 2;
    s32 bi = 0;
    for (const char *p = fmt; *p && bi < 4090; ++p) {
        if (*p == '%') {
            ++p; if (!*p) break;
            while (*p == '-' || *p == '+' || *p == ' ' || *p == '0' || *p == '#') ++p;
            while (*p >= '0' && *p <= '9') ++p;
            if (*p == '.') { ++p; while (*p >= '0' && *p <= '9') ++p; }
            if (*p == 'l') { ++p; if (*p == 'l') ++p; }
            switch (*p) {
                case 'd': case 'i': {
                    s64 v = (ai < num_args) ? val_to_int(args[ai++]) : 0;
                    bi += snprintf(buf + bi, 4096 - bi, "%lld", (long long)v);
                } break;
                case 's': {
                    const char *s = (ai < num_args && args[ai].kind == VAL_STRING) ? args[ai++].str : "";
                    bi += snprintf(buf + bi, 4096 - bi, "%s", s ? s : "");
                } break;
                case 'c': {
                    s64 v = (ai < num_args) ? val_to_int(args[ai++]) : 0;
                    buf[bi++] = (char)v;
                } break;
                case '%': buf[bi++] = '%'; break;
                default:  buf[bi++] = '%'; buf[bi++] = *p; break;
            }
        } else {
            buf[bi++] = *p;
        }
    }
    buf[bi] = '\0';

    if (args[0].kind == VAL_PTR && args[0].ptr && args[0].ptr->kind == VAL_STRING) {
        free(args[0].ptr->str);
        args[0].ptr->str = (char *)malloc(bi + 1);
        memcpy(args[0].ptr->str, buf, bi + 1);
    }
    return val_int(bi);
}

/* Builtin table */
global BuiltinEntry g_builtins[] = {
    { "printf",  builtin_printf  },
    { "putchar", builtin_putchar },
    { "puts",    builtin_puts    },
    { "malloc",  builtin_malloc  },
    { "calloc",  builtin_calloc  },
    { "free",    builtin_free    },
    { "exit",    builtin_exit    },
    { "abs",     builtin_abs     },
    { "strlen",  builtin_strlen  },
    { "strcmp",  builtin_strcmp  },
    { "atoi",    builtin_atoi    },
    { "sprintf", builtin_sprintf },
    { NULL, NULL }
};

internal BuiltinFn find_builtin(String name) {
    if (!name.data) return NULL;
    for (s32 i = 0; g_builtins[i].name; ++i) {
        s32 blen = (s32)strlen(g_builtins[i].name);
        if (blen == (s32)name.count && memcmp(g_builtins[i].name, name.data, name.count) == 0)
            return g_builtins[i].fn;
    }
    return NULL;
}

/* ======================================================================
 * Struct type registry — remembers struct/union field layouts
 * ====================================================================== */

typedef struct StructDef {
    String name;
    AggregateField *fields;
    s32 num_fields;
    b32 is_union;
} StructDef;

global StructDef *g_struct_defs = NULL;  /* DynamicArray */

internal StructDef *find_struct_def(String name) {
    s32 len = (s32)darr_len(g_struct_defs);
    for (s32 i = 0; i < len; ++i) {
        if (str_eq2(g_struct_defs[i].name, name))
            return &g_struct_defs[i];
    }
    return NULL;
}

internal void register_struct(String name, AggregateField *fields, s32 num_fields, b32 is_union) {
    if (find_struct_def(name)) return;
    StructDef def;
    def.name = name;
    def.fields = fields;
    def.num_fields = num_fields;
    def.is_union = is_union;
    darr_push(g_struct_defs, def);
}

/* Create a zero-initialized struct Val from a struct definition */
internal Val make_struct_val(String name) {
    StructDef *def = find_struct_def(name);
    if (!def) {
        Val v = val_struct(name, 0);
        return v;
    }
    Val v = val_struct(name, def->num_fields);
    for (s32 i = 0; i < def->num_fields; ++i) {
        v.strct.fields[i].name = def->fields[i].name;
        v.strct.fields[i].value = (Val *)calloc(1, sizeof(Val));
        *v.strct.fields[i].value = val_int(0);
    }
    return v;
}

/* ======================================================================
 * Typedef registry
 * ====================================================================== */

typedef struct TypedefEntry {
    String name;
    TypeSpec *type;
} TypedefEntry;

global TypedefEntry *g_typedefs = NULL;  /* DynamicArray */

internal TypeSpec *find_typedef(String name) {
    s32 len = (s32)darr_len(g_typedefs);
    for (s32 i = 0; i < len; ++i) {
        if (str_eq2(g_typedefs[i].name, name))
            return g_typedefs[i].type;
    }
    return NULL;
}

internal void register_typedef(String name, TypeSpec *type) {
    TypedefEntry e;
    e.name = name;
    e.type = type;
    darr_push(g_typedefs, e);
}

/* ======================================================================
 * Enum value registry
 * ====================================================================== */

typedef struct EnumVal {
    String name;
    s64 value;
} EnumVal;

global EnumVal *g_enum_vals = NULL;  /* DynamicArray */

internal s64 *find_enum_val(String name) {
    s32 len = (s32)darr_len(g_enum_vals);
    for (s32 i = 0; i < len; ++i) {
        if (str_eq2(g_enum_vals[i].name, name))
            return &g_enum_vals[i].value;
    }
    return NULL;
}

internal void register_enum_val(String name, s64 value) {
    EnumVal e;
    e.name = name;
    e.value = value;
    darr_push(g_enum_vals, e);
}

/* ======================================================================
 * Zero-value for a given type specification
 * ====================================================================== */

internal Val zero_val_for_type(TypeSpec *type) {
    if (!type) return val_int(0);

    /* Unwrap qualifiers */
    while (type->kind == TYPESPEC_CONST || type->kind == TYPESPEC_VOLATILE)
        type = type->ptr.base;

    switch (type->kind) {
        case TYPESPEC_NAME: {
            String n = type->name.name;
            if (!n.data) return val_int(0);
            if (str_eq(n, "float") || str_eq(n, "double"))
                return val_float(0.0);
            if (str_eq(n, "void"))
                return val_void();
            /* Check if it's a typedef'd struct */
            {
                StructDef *sd = find_struct_def(type->name.name);
                if (sd) return make_struct_val(type->name.name);
            }
            return val_int(0);
        }
        case TYPESPEC_PTR:
            return val_ptr(NULL);
        case TYPESPEC_ARRAY: {
            s32 size = 0;
            if (type->array.size) {
                /* Evaluate constant expression for array size */
                size = 8; /* default fallback */
            }
            return val_array(size);
        }
        case TYPESPEC_STRUCT:
        case TYPESPEC_UNION:
            return make_struct_val(type->aggregate.name);
        default:
            return val_int(0);
    }
}

/* ======================================================================
 * Interpreter initialization
 * ====================================================================== */

internal Interp interp_init(void) {
    Interp interp;
    memset(&interp, 0, sizeof(interp));
    interp.global_scope = scope_new(NULL);
    interp.current_scope = interp.global_scope;
    interp.flow = FLOW_NONE;
    interp.had_error = 0;

    /* Reset global registries */
    g_struct_defs = NULL;
    g_typedefs = NULL;
    g_enum_vals = NULL;

    return interp;
}

/* ======================================================================
 * L-value evaluation — returns a pointer to the storage location
 * ====================================================================== */

internal Val *eval_lvalue(Interp *interp, Expr *expr) {
    if (!expr) return NULL;

    switch (expr->kind) {
        case EXPR_NAME: {
            Val *v = scope_lookup(interp, expr->name.name);
            if (!v) {
                interp_error(interp, expr->line, "undefined variable '%.*s'",
                             (int)expr->name.name.count, expr->name.name.data);
                return NULL;
            }
            return v;
        }
        case EXPR_UNARY: {
            if (expr->unary.op == TK_Mul) {
                Val ptr = eval_expr(interp, expr->unary.operand);
                if (ptr.kind == VAL_PTR) return ptr.ptr;
                interp_error(interp, expr->line, "dereference of non-pointer");
                return NULL;
            }
            break;
        }
        case EXPR_INDEX: {
            Val *arr_lv = eval_lvalue(interp, expr->index.array);
            if (!arr_lv) return NULL;
            Val idx = eval_expr(interp, expr->index.index);
            s64 i = val_to_int(idx);
            if (arr_lv->kind == VAL_ARRAY) {
                if (i < 0 || i >= arr_lv->arr.len) {
                    interp_error(interp, expr->line, "array index %lld out of bounds (len=%d)",
                                 (long long)i, arr_lv->arr.len);
                    return NULL;
                }
                return &arr_lv->arr.elems[i];
            } else if (arr_lv->kind == VAL_PTR && arr_lv->ptr) {
                /* Pointer arithmetic: treat as array */
                return arr_lv->ptr + i;
            }
            interp_error(interp, expr->line, "subscript on non-array/non-pointer");
            return NULL;
        }
        case EXPR_FIELD: {
            Val *obj = eval_lvalue(interp, expr->field.object);
            if (!obj || obj->kind != VAL_STRUCT) {
                interp_error(interp, expr->line, "member access on non-struct");
                return NULL;
            }
            for (s32 i = 0; i < obj->strct.num_fields; ++i) {
                if (str_eq2(obj->strct.fields[i].name, expr->field.field_name))
                    return obj->strct.fields[i].value;
            }
            interp_error(interp, expr->line, "no field '%.*s' in struct '%.*s'",
                         (int)expr->field.field_name.count, expr->field.field_name.data,
                         (int)obj->strct.name.count, obj->strct.name.data);
            return NULL;
        }
        case EXPR_FIELD_PTR: {
            Val ptr = eval_expr(interp, expr->field.object);
            Val *obj = NULL;
            if (ptr.kind == VAL_PTR) obj = ptr.ptr;
            if (!obj || obj->kind != VAL_STRUCT) {
                interp_error(interp, expr->line, "'->' on non-struct-pointer");
                return NULL;
            }
            for (s32 i = 0; i < obj->strct.num_fields; ++i) {
                if (str_eq2(obj->strct.fields[i].name, expr->field.field_name))
                    return obj->strct.fields[i].value;
            }
            interp_error(interp, expr->line, "no field '%.*s' via '->'",
                         (int)expr->field.field_name.count, expr->field.field_name.data);
            return NULL;
        }
        case EXPR_PAREN:
            return eval_lvalue(interp, expr->paren.inner);
        default:
            break;
    }
    interp_error(interp, expr->line, "expression is not an lvalue");
    return NULL;
}

/* ======================================================================
 * Expression evaluation
 * ====================================================================== */

internal Val eval_expr(Interp *interp, Expr *expr) {
    if (!expr) return val_int(0);
    if (interp->had_error) return val_int(0);

    switch (expr->kind) {
        case EXPR_INT:
            return val_int((s64)expr->int_lit.val);

        case EXPR_FLOAT:
            return val_float(expr->float_lit.val);

        case EXPR_STR: {
            const char *raw = expr->str_lit.val.data;
            s32 raw_len = (s32)expr->str_lit.val.count;

            /* Strip surrounding quotes if present */
            if (raw_len >= 2 && raw[0] == '"') {
                raw++; raw_len -= 2;
            }

            /* Process escape sequences */
            char *s = (char *)malloc(raw_len + 1);
            s32 out = 0;
            for (s32 i = 0; i < raw_len; ++i) {
                if (raw[i] == '\\' && i + 1 < raw_len) {
                    ++i;
                    switch (raw[i]) {
                        case 'n':  s[out++] = '\n'; break;
                        case 't':  s[out++] = '\t'; break;
                        case 'r':  s[out++] = '\r'; break;
                        case '\\': s[out++] = '\\'; break;
                        case '\'': s[out++] = '\''; break;
                        case '"':  s[out++] = '"';  break;
                        case '0':  s[out++] = '\0'; break;
                        case 'a':  s[out++] = '\a'; break;
                        case 'b':  s[out++] = '\b'; break;
                        case 'f':  s[out++] = '\f'; break;
                        case 'v':  s[out++] = '\v'; break;
                        default:   s[out++] = '\\'; s[out++] = raw[i]; break;
                    }
                } else {
                    s[out++] = raw[i];
                }
            }
            s[out] = '\0';
            return val_string(s);
        }

        case EXPR_NAME: {
            /* Check enum values first */
            s64 *ev = find_enum_val(expr->name.name);
            if (ev) return val_int(*ev);

            Val *v = scope_lookup(interp, expr->name.name);
            if (v) return *v;
            /* Check builtins */
            if (expr->name.name.data) {
                BuiltinFn bfn = find_builtin(expr->name.name);
                if (bfn) {
                    /* Return a sentinel — function calls handle this */
                    return val_int(0);
                }
            }
            interp_error(interp, expr->line, "undefined variable '%.*s'",
                         (int)expr->name.name.count, expr->name.name.data);
            return val_int(0);
        }

        case EXPR_PAREN:
            return eval_expr(interp, expr->paren.inner);

        case EXPR_UNARY: {
            switch (expr->unary.op) {
                case TK_Minus: {
                    Val v = eval_expr(interp, expr->unary.operand);
                    if (v.kind == VAL_FLOAT) return val_float(-v.f);
                    return val_int(-val_to_int(v));
                }
                case TK_Plus:
                    return eval_expr(interp, expr->unary.operand);
                case TK_Not: {
                    Val v = eval_expr(interp, expr->unary.operand);
                    return val_int(!val_is_truthy(v));
                }
                case TK_Tilde: {
                    Val v = eval_expr(interp, expr->unary.operand);
                    return val_int(~val_to_int(v));
                }
                case TK_Mul: {
                    Val v = eval_expr(interp, expr->unary.operand);
                    if (v.kind == VAL_PTR && v.ptr) return *v.ptr;
                    interp_error(interp, expr->line, "dereference of non-pointer or NULL");
                    return val_int(0);
                }
                case TK_And: {
                    Val *lv = eval_lvalue(interp, expr->unary.operand);
                    if (lv) return val_ptr(lv);
                    return val_ptr(NULL);
                }
                case TK_Inc: {
                    Val *lv = eval_lvalue(interp, expr->unary.operand);
                    if (lv) {
                        if (lv->kind == VAL_FLOAT) { lv->f += 1.0; return *lv; }
                        lv->i += 1;
                        return *lv;
                    }
                    return val_int(0);
                }
                case TK_Dec: {
                    Val *lv = eval_lvalue(interp, expr->unary.operand);
                    if (lv) {
                        if (lv->kind == VAL_FLOAT) { lv->f -= 1.0; return *lv; }
                        lv->i -= 1;
                        return *lv;
                    }
                    return val_int(0);
                }
                default:
                    interp_error(interp, expr->line, "unhandled unary operator");
                    return val_int(0);
            }
        }

        case EXPR_POSTFIX: {
            Val *lv = eval_lvalue(interp, expr->unary.operand);
            if (!lv) return val_int(0);
            Val old = *lv;
            if (expr->unary.op == TK_Inc) {
                if (lv->kind == VAL_FLOAT) lv->f += 1.0;
                else lv->i += 1;
            } else {
                if (lv->kind == VAL_FLOAT) lv->f -= 1.0;
                else lv->i -= 1;
            }
            return old;
        }

        case EXPR_BINARY: {
            TokenKind op = expr->binary.op;

            /* Short-circuit operators */
            if (op == TK_AndAnd) {
                Val l = eval_expr(interp, expr->binary.left);
                if (!val_is_truthy(l)) return val_int(0);
                Val r = eval_expr(interp, expr->binary.right);
                return val_int(val_is_truthy(r) ? 1 : 0);
            }
            if (op == TK_OrOr) {
                Val l = eval_expr(interp, expr->binary.left);
                if (val_is_truthy(l)) return val_int(1);
                Val r = eval_expr(interp, expr->binary.right);
                return val_int(val_is_truthy(r) ? 1 : 0);
            }

            /* Assignment operators */
            if (op == TK_Asgn || op == TK_PlusEq || op == TK_MinusEq ||
                op == TK_MulEq || op == TK_DivEq || op == TK_ModEq ||
                op == TK_AndEq || op == TK_OrEq || op == TK_XorEq ||
                op == TK_ShiftLEq || op == TK_ShiftREq) {

                Val *lv = eval_lvalue(interp, expr->binary.left);
                if (!lv) return val_int(0);
                Val rhs = eval_expr(interp, expr->binary.right);

                if (op == TK_Asgn) {
                    *lv = rhs;
                    return *lv;
                }

                /* Compute for compound assignments */
                b32 use_float = (lv->kind == VAL_FLOAT || rhs.kind == VAL_FLOAT);

                if (use_float) {
                    f64 l = val_to_float(*lv);
                    f64 r = val_to_float(rhs);
                    switch (op) {
                        case TK_PlusEq:  l += r; break;
                        case TK_MinusEq: l -= r; break;
                        case TK_MulEq:   l *= r; break;
                        case TK_DivEq:   l = (r != 0.0) ? l / r : 0.0; break;
                        default: break;
                    }
                    *lv = val_float(l);
                } else {
                    s64 l = val_to_int(*lv);
                    s64 r = val_to_int(rhs);
                    switch (op) {
                        case TK_PlusEq:       l += r; break;
                        case TK_MinusEq:      l -= r; break;
                        case TK_MulEq:        l *= r; break;
                        case TK_DivEq:        l = (r != 0) ? l / r : 0; break;
                        case TK_ModEq:        l = (r != 0) ? l % r : 0; break;
                        case TK_AndEq:        l &= r; break;
                        case TK_OrEq:         l |= r; break;
                        case TK_XorEq:        l ^= r; break;
                        case TK_ShiftLEq:  l <<= r; break;
                        case TK_ShiftREq: l >>= r; break;
                        default: break;
                    }
                    *lv = val_int(l);
                }
                return *lv;
            }

            /* Comma operator */
            if (op == TK_Comma) {
                eval_expr(interp, expr->binary.left);
                return eval_expr(interp, expr->binary.right);
            }

            /* Arithmetic / comparison / bitwise */
            Val lv = eval_expr(interp, expr->binary.left);
            Val rv = eval_expr(interp, expr->binary.right);

            /* Pointer arithmetic: ptr + int, ptr - int */
            if (lv.kind == VAL_PTR && rv.kind == VAL_INT) {
                if (op == TK_Plus)  return val_ptr(lv.ptr + rv.i);
                if (op == TK_Minus) return val_ptr(lv.ptr - rv.i);
            }

            b32 use_float = (lv.kind == VAL_FLOAT || rv.kind == VAL_FLOAT);

            if (use_float) {
                f64 l = val_to_float(lv);
                f64 r = val_to_float(rv);
                switch (op) {
                    case TK_Plus:       return val_float(l + r);
                    case TK_Minus:      return val_float(l - r);
                    case TK_Mul:       return val_float(l * r);
                    case TK_Div:      return val_float((r != 0.0) ? l / r : 0.0);
                    case TK_Less:       return val_int(l < r);
                    case TK_LessEq:  return val_int(l <= r);
                    case TK_Grt:    return val_int(l > r);
                    case TK_GrtEq: return val_int(l >= r);
                    case TK_Eq: return val_int(l == r);
                    case TK_NotEq:   return val_int(l != r);
                    default: break;
                }
            }

            s64 l = val_to_int(lv);
            s64 r = val_to_int(rv);
            switch (op) {
                case TK_Plus:         return val_int(l + r);
                case TK_Minus:        return val_int(l - r);
                case TK_Mul:         return val_int(l * r);
                case TK_Div:        return val_int((r != 0) ? l / r : 0);
                case TK_Mod:       return val_int((r != 0) ? l % r : 0);
                case TK_And:    return val_int(l & r);
                case TK_Or:         return val_int(l | r);
                case TK_Xor:        return val_int(l ^ r);
                case TK_ShiftL:    return val_int(l << r);
                case TK_ShiftR:   return val_int(l >> r);
                case TK_Less:         return val_int(l < r);
                case TK_LessEq:    return val_int(l <= r);
                case TK_Grt:      return val_int(l > r);
                case TK_GrtEq: return val_int(l >= r);
                case TK_Eq:   return val_int(l == r);
                case TK_NotEq:     return val_int(l != r);
                default:
                    interp_error(interp, expr->line, "unhandled binary operator");
                    return val_int(0);
            }
        }

        case EXPR_TERNARY: {
            Val cond = eval_expr(interp, expr->ternary.cond);
            if (val_is_truthy(cond))
                return eval_expr(interp, expr->ternary.then_expr);
            else
                return eval_expr(interp, expr->ternary.else_expr);
        }

        case EXPR_CALL: {
            /* Resolve function name */
            if (expr->call.func->kind == EXPR_NAME) {
                String fn_name = expr->call.func->name.name;

                /* Evaluate arguments */
                s32 nargs = expr->call.num_args;
                Val *args = NULL;
                if (nargs > 0) {
                    args = (Val *)malloc(nargs * sizeof(Val));
                    for (s32 i = 0; i < nargs; ++i)
                        args[i] = eval_expr(interp, expr->call.args[i]);
                }

                /* Check builtins */
                if (fn_name.data) {
                    BuiltinFn bfn = find_builtin(fn_name);
                    if (bfn) {
                        Val result = bfn(interp, args, nargs);
                        if (args) free(args);
                        return result;
                    }
                }

                /* Check user-defined functions */
                Val *fval = scope_lookup(interp, fn_name);
                if (fval && fval->kind == VAL_FUNC && fval->func) {
                    Val result = call_function(interp, fval->func, args, nargs);
                    if (args) free(args);
                    return result;
                }

                if (args) free(args);
                interp_error(interp, expr->line, "undefined function '%.*s'",
                             (int)fn_name.count, fn_name.data);
                return val_int(0);
            }
            /* Indirect function call through pointer/expression */
            Val fn = eval_expr(interp, expr->call.func);
            if (fn.kind == VAL_FUNC && fn.func) {
                s32 nargs = expr->call.num_args;
                Val *args = NULL;
                if (nargs > 0) {
                    args = (Val *)malloc(nargs * sizeof(Val));
                    for (s32 i = 0; i < nargs; ++i)
                        args[i] = eval_expr(interp, expr->call.args[i]);
                }
                Val result = call_function(interp, fn.func, args, nargs);
                if (args) free(args);
                return result;
            }
            interp_error(interp, expr->line, "call to non-function");
            return val_int(0);
        }

        case EXPR_INDEX: {
            Val arr = eval_expr(interp, expr->index.array);
            Val idx = eval_expr(interp, expr->index.index);
            s64 i = val_to_int(idx);
            if (arr.kind == VAL_ARRAY) {
                if (i < 0 || i >= arr.arr.len) {
                    interp_error(interp, expr->line, "index %lld out of bounds", (long long)i);
                    return val_int(0);
                }
                return arr.arr.elems[i];
            }
            if (arr.kind == VAL_PTR && arr.ptr) {
                return *(arr.ptr + i);
            }
            if (arr.kind == VAL_STRING && arr.str) {
                if (i >= 0 && i < (s64)strlen(arr.str))
                    return val_int((s64)arr.str[i]);
                return val_int(0);
            }
            interp_error(interp, expr->line, "subscript on non-array");
            return val_int(0);
        }

        case EXPR_FIELD: {
            Val *obj = eval_lvalue(interp, expr->field.object);
            if (!obj || obj->kind != VAL_STRUCT) {
                interp_error(interp, expr->line, "'.' on non-struct");
                return val_int(0);
            }
            for (s32 i = 0; i < obj->strct.num_fields; ++i) {
                if (str_eq2(obj->strct.fields[i].name, expr->field.field_name))
                    return *obj->strct.fields[i].value;
            }
            interp_error(interp, expr->line, "no field '%.*s'",
                         (int)expr->field.field_name.count, expr->field.field_name.data);
            return val_int(0);
        }

        case EXPR_FIELD_PTR: {
            Val ptr = eval_expr(interp, expr->field.object);
            Val *obj = NULL;
            if (ptr.kind == VAL_PTR) obj = ptr.ptr;
            if (!obj || obj->kind != VAL_STRUCT) {
                interp_error(interp, expr->line, "'->' on non-struct-pointer");
                return val_int(0);
            }
            for (s32 i = 0; i < obj->strct.num_fields; ++i) {
                if (str_eq2(obj->strct.fields[i].name, expr->field.field_name))
                    return *obj->strct.fields[i].value;
            }
            interp_error(interp, expr->line, "no field '%.*s' via '->'",
                         (int)expr->field.field_name.count, expr->field.field_name.data);
            return val_int(0);
        }

        case EXPR_CAST: {
            Val v = eval_expr(interp, expr->cast.expr);
            /* Simplified cast: int<->float, everything else passthrough */
            TypeSpec *t = expr->cast.type;
            while (t && (t->kind == TYPESPEC_CONST || t->kind == TYPESPEC_VOLATILE))
                t = t->ptr.base;
            if (t && t->kind == TYPESPEC_NAME && t->name.name.data) {
                if (str_eq(t->name.name, "int") || str_eq(t->name.name, "long") ||
                    str_eq(t->name.name, "short") || str_eq(t->name.name, "char"))
                    return val_int(val_to_int(v));
                if (str_eq(t->name.name, "float") || str_eq(t->name.name, "double"))
                    return val_float(val_to_float(v));
            }
            if (t && t->kind == TYPESPEC_PTR) {
                if (v.kind == VAL_INT) return val_ptr((Val *)(intptr_t)v.i);
                return v;
            }
            return v;
        }

        case EXPR_SIZEOF_EXPR: {
            Val v = eval_expr(interp, expr->sizeof_expr.expr);
            switch (v.kind) {
                case VAL_INT:    return val_int(4);
                case VAL_FLOAT:  return val_int(8);
                case VAL_PTR:    return val_int(8);
                case VAL_STRING: return val_int(v.str ? (s64)strlen(v.str) + 1 : 0);
                case VAL_ARRAY:  return val_int(v.arr.len * 4);
                default:         return val_int(0);
            }
        }

        case EXPR_SIZEOF_TYPE:
            return val_int(type_sizeof(expr->sizeof_type.type));

        case EXPR_COMPOUND: {
            s32 n = expr->compound.num_elems;
            Val arr = val_array(n);
            for (s32 i = 0; i < n; ++i)
                arr.arr.elems[i] = eval_expr(interp, expr->compound.elems[i]);
            return arr;
        }

        default:
            interp_error(interp, expr->line, "unhandled expression kind %d", expr->kind);
            return val_int(0);
    }
}

/* ======================================================================
 * Statement execution
 * ====================================================================== */

internal void exec_block_stmts(Interp *interp, Stmt **stmts, s32 num_stmts);

internal void exec_stmt(Interp *interp, Stmt *stmt) {
    if (!stmt) return;
    if (interp->had_error) return;
    if (interp->flow != FLOW_NONE) return;

    switch (stmt->kind) {
        case STMT_EXPR:
            eval_expr(interp, stmt->expr.expr);
            break;

        case STMT_RETURN:
            interp->return_val = stmt->ret.expr ? eval_expr(interp, stmt->ret.expr) : val_void();
            interp->flow = FLOW_RETURN;
            break;

        case STMT_IF: {
            Val cond = eval_expr(interp, stmt->if_stmt.cond);
            if (val_is_truthy(cond))
                exec_stmt(interp, stmt->if_stmt.then_body);
            else if (stmt->if_stmt.else_body)
                exec_stmt(interp, stmt->if_stmt.else_body);
            break;
        }

        case STMT_WHILE: {
            while (interp->flow == FLOW_NONE || interp->flow == FLOW_CONTINUE) {
                if (interp->flow == FLOW_CONTINUE) interp->flow = FLOW_NONE;
                Val cond = eval_expr(interp, stmt->while_stmt.cond);
                if (!val_is_truthy(cond)) break;
                exec_stmt(interp, stmt->while_stmt.body);
                if (interp->flow == FLOW_BREAK) { interp->flow = FLOW_NONE; break; }
                if (interp->flow == FLOW_RETURN) break;
            }
            if (interp->flow == FLOW_CONTINUE) interp->flow = FLOW_NONE;
            break;
        }

        case STMT_DO_WHILE: {
            do {
                if (interp->flow == FLOW_CONTINUE) interp->flow = FLOW_NONE;
                exec_stmt(interp, stmt->do_while_stmt.body);
                if (interp->flow == FLOW_BREAK) { interp->flow = FLOW_NONE; break; }
                if (interp->flow == FLOW_RETURN) break;
                Val cond = eval_expr(interp, stmt->do_while_stmt.cond);
                if (!val_is_truthy(cond)) break;
            } while (1);
            if (interp->flow == FLOW_CONTINUE) interp->flow = FLOW_NONE;
            break;
        }

        case STMT_FOR: {
            scope_push(interp);

            if (stmt->for_stmt.init)
                exec_stmt(interp, stmt->for_stmt.init);

            while (interp->flow == FLOW_NONE || interp->flow == FLOW_CONTINUE) {
                if (interp->flow == FLOW_CONTINUE) interp->flow = FLOW_NONE;

                if (stmt->for_stmt.cond) {
                    Val cond = eval_expr(interp, stmt->for_stmt.cond);
                    if (!val_is_truthy(cond)) break;
                }

                exec_stmt(interp, stmt->for_stmt.body);

                if (interp->flow == FLOW_BREAK) { interp->flow = FLOW_NONE; break; }
                if (interp->flow == FLOW_RETURN) break;
                if (interp->flow == FLOW_CONTINUE) interp->flow = FLOW_NONE;

                if (stmt->for_stmt.post)
                    eval_expr(interp, stmt->for_stmt.post);
            }

            scope_pop(interp);
            break;
        }

        case STMT_SWITCH: {
            Val sw = eval_expr(interp, stmt->switch_stmt.expr);
            s64 sw_val = val_to_int(sw);
            s32 nc = stmt->switch_stmt.num_cases;
            b32 matched = 0;
            s32 start_case = -1;

            /* Find matching case */
            for (s32 i = 0; i < nc; ++i) {
                if (!stmt->switch_stmt.cases[i].is_default && stmt->switch_stmt.cases[i].value) {
                    Val cv = eval_expr(interp, stmt->switch_stmt.cases[i].value);
                    if (val_to_int(cv) == sw_val) {
                        start_case = i;
                        matched = 1;
                        break;
                    }
                }
            }
            /* If no match, find default */
            if (!matched) {
                for (s32 i = 0; i < nc; ++i) {
                    if (stmt->switch_stmt.cases[i].is_default) {
                        start_case = i;
                        matched = 1;
                        break;
                    }
                }
            }

            /* Execute from matched case, fall through */
            if (matched) {
                for (s32 i = start_case; i < nc; ++i) {
                    SwitchCase *sc = &stmt->switch_stmt.cases[i];
                    for (s32 j = 0; j < sc->num_stmts; ++j) {
                        exec_stmt(interp, sc->stmts[j]);
                        if (interp->flow == FLOW_BREAK) {
                            interp->flow = FLOW_NONE;
                            goto switch_done;
                        }
                        if (interp->flow == FLOW_RETURN) goto switch_done;
                    }
                }
            }
            switch_done:
            break;
        }

        case STMT_BREAK:
            interp->flow = FLOW_BREAK;
            break;

        case STMT_CONTINUE:
            interp->flow = FLOW_CONTINUE;
            break;

        case STMT_GOTO:
            interp->goto_label = stmt->goto_stmt.label;
            interp->flow = FLOW_GOTO;
            break;

        case STMT_LABEL: {
            /* Labels are handled during block execution scan */
            exec_stmt(interp, stmt->label_stmt.stmt);
            break;
        }

        case STMT_BLOCK: {
            scope_push(interp);
            exec_block_stmts(interp, stmt->block.stmts, stmt->block.num_stmts);
            scope_pop(interp);
            break;
        }

        case STMT_DECL:
            exec_decl(interp, stmt->decl.decl);
            break;

        default:
            interp_error(interp, stmt->line, "unhandled statement kind %d", stmt->kind);
            break;
    }
}

/* Execute a sequence of statements, handling goto by scanning for labels */
internal void exec_block_stmts(Interp *interp, Stmt **stmts, s32 num_stmts) {
    s32 start = 0;

restart:
    for (s32 i = start; i < num_stmts; ++i) {
        exec_stmt(interp, stmts[i]);

        if (interp->flow == FLOW_GOTO) {
            /* Scan for the label in this block */
            for (s32 j = 0; j < num_stmts; ++j) {
                if (stmts[j]->kind == STMT_LABEL &&
                    str_eq2(stmts[j]->label_stmt.label, interp->goto_label)) {
                    interp->flow = FLOW_NONE;
                    start = j;
                    goto restart;
                }
            }
            return; /* Label not in this block, propagate */
        }

        if (interp->flow != FLOW_NONE) return;
    }
}

/* ======================================================================
 * Declaration execution
 * ====================================================================== */

internal Val resolve_init_for_type(Interp *interp, TypeSpec *type, Expr *init);

internal Val resolve_init_for_type(Interp *interp, TypeSpec *type, Expr *init) {
    if (init) return eval_expr(interp, init);
    return zero_val_for_type(type);
}

internal void exec_decl(Interp *interp, ASTDecl *decl) {
    if (!decl) return;
    if (interp->had_error) return;

    switch (decl->kind) {
        case DECL_VAR: {
            Val init = resolve_init_for_type(interp, decl->var.type, decl->var.init);

            /* Special case: pointer/array variable with struct type */
            TypeSpec *t = decl->var.type;
            while (t && (t->kind == TYPESPEC_CONST || t->kind == TYPESPEC_VOLATILE))
                t = t->ptr.base;
            if (t && t->kind == TYPESPEC_STRUCT && !decl->var.init) {
                init = make_struct_val(t->aggregate.name);
            }
            if (t && t->kind == TYPESPEC_NAME && !decl->var.init) {
                StructDef *sd = find_struct_def(t->name.name);
                if (sd) init = make_struct_val(t->name.name);
            }

            scope_define(interp->current_scope, decl->var.name, init);
            break;
        }

        case DECL_FUNC: {
            if (decl->storage == STORAGE_EXTERN) break;
            scope_define(interp->global_scope, decl->func.name, val_func(decl));
            break;
        }

        case DECL_STRUCT:
            register_struct(decl->aggregate.name, decl->aggregate.fields,
                            decl->aggregate.num_fields, 0);
            break;

        case DECL_UNION:
            register_struct(decl->aggregate.name, decl->aggregate.fields,
                            decl->aggregate.num_fields, 1);
            break;

        case DECL_ENUM: {
            s64 next_val = 0;
            for (s32 i = 0; i < decl->enumeration.num_items; ++i) {
                EnumItem *item = &decl->enumeration.items[i];
                if (item->value) {
                    Val v = eval_expr(interp, item->value);
                    next_val = val_to_int(v);
                }
                register_enum_val(item->name, next_val);
                scope_define(interp->global_scope, item->name, val_int(next_val));
                next_val++;
            }
            break;
        }

        case DECL_TYPEDEF:
            register_typedef(decl->type_def.name, decl->type_def.type);
            break;

        default:
            break;
    }
}

/* ======================================================================
 * Function calls
 * ====================================================================== */

internal Val call_function(Interp *interp, ASTDecl *func, Val *args, s32 num_args) {
    if (!func || func->kind != DECL_FUNC) {
        interp_error(interp, 0, "call to non-function declaration");
        return val_int(0);
    }

    if (!func->func.body) {
        interp_error(interp, func->line, "call to forward-declared function '%.*s' with no body",
                     (int)func->func.name.count, func->func.name.data);
        return val_int(0);
    }

    /* Push a new scope for the function body */
    scope_push(interp);

    /* Bind parameters */
    s32 num_params = func->func.num_params;
    for (s32 i = 0; i < num_params && i < num_args; ++i) {
        scope_define(interp->current_scope, func->func.params[i].name, args[i]);
    }
    /* If fewer args than params, zero-fill */
    for (s32 i = num_args; i < num_params; ++i) {
        scope_define(interp->current_scope, func->func.params[i].name,
                     zero_val_for_type(func->func.params[i].type));
    }

    /* Execute body */
    FlowKind saved_flow = interp->flow;
    Val saved_ret = interp->return_val;
    interp->flow = FLOW_NONE;
    interp->return_val = val_void();

    exec_stmt(interp, func->func.body);

    Val result = interp->return_val;
    if (interp->flow == FLOW_RETURN)
        interp->flow = FLOW_NONE;

    interp->return_val = saved_ret;
    if (interp->flow == FLOW_NONE)
        interp->flow = saved_flow;

    scope_pop(interp);
    return result;
}

/* ======================================================================
 * Top-level: execute a translation unit
 * ====================================================================== */

internal void interp_run(Interp *interp, ASTDecl **decls) {
    s32 num_decls = (s32)darr_len(decls);

    /* Pass 1: Register all types and functions (forward declarations work) */
    for (s32 i = 0; i < num_decls; ++i) {
        ASTDecl *d = decls[i];
        switch (d->kind) {
            case DECL_STRUCT:
                register_struct(d->aggregate.name, d->aggregate.fields, d->aggregate.num_fields, 0);
                break;
            case DECL_UNION:
                register_struct(d->aggregate.name, d->aggregate.fields, d->aggregate.num_fields, 1);
                break;
            case DECL_ENUM: {
                s64 next_val = 0;
                for (s32 j = 0; j < d->enumeration.num_items; ++j) {
                    EnumItem *item = &d->enumeration.items[j];
                    if (item->value) {
                        Val v = eval_expr(interp, item->value);
                        next_val = val_to_int(v);
                    }
                    register_enum_val(item->name, next_val);
                    scope_define(interp->global_scope, item->name, val_int(next_val));
                    next_val++;
                }
                break;
            }
            case DECL_TYPEDEF:
                register_typedef(d->type_def.name, d->type_def.type);
                break;
            case DECL_FUNC:
                scope_define(interp->global_scope, d->func.name, val_func(d));
                break;
            default: break;
        }
    }

    /* Pass 2: Process global variable declarations */
    for (s32 i = 0; i < num_decls; ++i) {
        ASTDecl *d = decls[i];
        if (d->kind == DECL_VAR && d->storage != STORAGE_EXTERN) {
            Val init = resolve_init_for_type(interp, d->var.type, d->var.init);

            TypeSpec *t = d->var.type;
            while (t && (t->kind == TYPESPEC_CONST || t->kind == TYPESPEC_VOLATILE))
                t = t->ptr.base;
            if (t && t->kind == TYPESPEC_STRUCT && !d->var.init)
                init = make_struct_val(t->aggregate.name);
            if (t && t->kind == TYPESPEC_NAME && !d->var.init) {
                StructDef *sd = find_struct_def(t->name.name);
                if (sd) init = make_struct_val(t->name.name);
            }

            scope_define(interp->global_scope, d->var.name, init);
        }
    }

    /* Pass 3: Find and call main() */
    Val *main_fn = scope_lookup(interp, (String){"main", 4, 0});
    if (!main_fn || main_fn->kind != VAL_FUNC) {
        interp_error(interp, 0, "no 'main' function found");
        return;
    }

    /* Call main with (argc=0, argv=NULL) — simplified */
    Val main_args[2];
    main_args[0] = val_int(0);
    main_args[1] = val_ptr(NULL);

    Val result = call_function(interp, main_fn->func, main_args, 2);
    (void)result;
}
