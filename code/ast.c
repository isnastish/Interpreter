
/*
 * ast.c — AST node constructors and pretty-printer.
 *
 * Every constructor allocates a node on the heap via malloc, sets the kind
 * tag and fields, then returns a pointer.  The pretty-printer walks the
 * tree recursively and prints an indented textual representation.
 */

/* ======================================================================
 * Internal helpers
 * ====================================================================== */

#define AST_ALLOC(type) ((type *)calloc(1, sizeof(type)))

internal void ast_indent(s32 level){
    for(s32 i = 0; i < level; ++i) printf("  ");
}

/* ======================================================================
 * TypeSpec constructors
 * ====================================================================== */

internal TypeSpec *typespec_name(String name){
    TypeSpec *t = AST_ALLOC(TypeSpec);
    t->kind = TYPESPEC_NAME;
    t->name.name = name;
    return t;
}

internal TypeSpec *typespec_ptr(TypeSpec *base){
    TypeSpec *t = AST_ALLOC(TypeSpec);
    t->kind = TYPESPEC_PTR;
    t->ptr.base = base;
    return t;
}

internal TypeSpec *typespec_array(TypeSpec *elem, Expr *size){
    TypeSpec *t = AST_ALLOC(TypeSpec);
    t->kind = TYPESPEC_ARRAY;
    t->array.elem = elem;
    t->array.size = size;
    return t;
}

internal TypeSpec *typespec_func(TypeSpec *ret, TypeSpec **params, b32 has_varargs){
    TypeSpec *t = AST_ALLOC(TypeSpec);
    t->kind = TYPESPEC_FUNC;
    t->func.ret = ret;
    t->func.params = params;
    t->func.has_varargs = has_varargs;
    return t;
}

internal TypeSpec *typespec_const(TypeSpec *base){
    TypeSpec *t = AST_ALLOC(TypeSpec);
    t->kind = TYPESPEC_CONST;
    t->ptr.base = base;
    return t;
}

internal TypeSpec *typespec_volatile(TypeSpec *base){
    TypeSpec *t = AST_ALLOC(TypeSpec);
    t->kind = TYPESPEC_VOLATILE;
    t->ptr.base = base;
    return t;
}

/* ======================================================================
 * Expr constructors
 * ====================================================================== */

internal Expr *expr_int(u64 val, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_INT;
    e->line = line; e->column = col;
    e->int_lit.val = val;
    return e;
}

internal Expr *expr_float(f64 val, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_FLOAT;
    e->line = line; e->column = col;
    e->float_lit.val = val;
    return e;
}

internal Expr *expr_str(String val, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_STR;
    e->line = line; e->column = col;
    e->str_lit.val = val;
    return e;
}

internal Expr *expr_name(String name, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_NAME;
    e->line = line; e->column = col;
    e->name.name = name;
    return e;
}

internal Expr *expr_unary(TokenKind op, Expr *operand, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_UNARY;
    e->line = line; e->column = col;
    e->unary.op = op;
    e->unary.operand = operand;
    return e;
}

internal Expr *expr_postfix(TokenKind op, Expr *operand, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_POSTFIX;
    e->line = line; e->column = col;
    e->unary.op = op;
    e->unary.operand = operand;
    return e;
}

internal Expr *expr_binary(TokenKind op, Expr *left, Expr *right, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_BINARY;
    e->line = line; e->column = col;
    e->binary.op = op;
    e->binary.left = left;
    e->binary.right = right;
    return e;
}

internal Expr *expr_ternary(Expr *cond, Expr *then_expr, Expr *else_expr, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_TERNARY;
    e->line = line; e->column = col;
    e->ternary.cond = cond;
    e->ternary.then_expr = then_expr;
    e->ternary.else_expr = else_expr;
    return e;
}

internal Expr *expr_call(Expr *func, Expr **args, s32 num_args, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_CALL;
    e->line = line; e->column = col;
    e->call.func = func;
    e->call.args = args;
    e->call.num_args = num_args;
    return e;
}

internal Expr *expr_index(Expr *array, Expr *index, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_INDEX;
    e->line = line; e->column = col;
    e->index.array = array;
    e->index.index = index;
    return e;
}

internal Expr *expr_field(Expr *object, String field_name, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_FIELD;
    e->line = line; e->column = col;
    e->field.object = object;
    e->field.field_name = field_name;
    return e;
}

internal Expr *expr_field_ptr(Expr *object, String field_name, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_FIELD_PTR;
    e->line = line; e->column = col;
    e->field.object = object;
    e->field.field_name = field_name;
    return e;
}

internal Expr *expr_cast(TypeSpec *type, Expr *inner, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_CAST;
    e->line = line; e->column = col;
    e->cast.type = type;
    e->cast.expr = inner;
    return e;
}

internal Expr *expr_sizeof_expr(Expr *inner, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_SIZEOF_EXPR;
    e->line = line; e->column = col;
    e->sizeof_expr.expr = inner;
    return e;
}

internal Expr *expr_sizeof_type(TypeSpec *type, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_SIZEOF_TYPE;
    e->line = line; e->column = col;
    e->sizeof_type.type = type;
    return e;
}

internal Expr *expr_paren(Expr *inner, u32 line, u32 col){
    Expr *e = AST_ALLOC(Expr);
    e->kind = EXPR_PAREN;
    e->line = line; e->column = col;
    e->paren.inner = inner;
    return e;
}

/* ======================================================================
 * Stmt constructors
 * ====================================================================== */

internal Stmt *stmt_expr(Expr *expression, u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_EXPR;
    s->line = line; s->column = col;
    s->expr.expr = expression;
    return s;
}

internal Stmt *stmt_return(Expr *expression, u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_RETURN;
    s->line = line; s->column = col;
    s->ret.expr = expression;
    return s;
}

internal Stmt *stmt_if(Expr *cond, Stmt *then_body, Stmt *else_body, u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_IF;
    s->line = line; s->column = col;
    s->if_stmt.cond = cond;
    s->if_stmt.then_body = then_body;
    s->if_stmt.else_body = else_body;
    return s;
}

internal Stmt *stmt_while(Expr *cond, Stmt *body, u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_WHILE;
    s->line = line; s->column = col;
    s->while_stmt.cond = cond;
    s->while_stmt.body = body;
    return s;
}

internal Stmt *stmt_do_while(Stmt *body, Expr *cond, u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_DO_WHILE;
    s->line = line; s->column = col;
    s->do_while_stmt.body = body;
    s->do_while_stmt.cond = cond;
    return s;
}

internal Stmt *stmt_for(Stmt *init, Expr *cond, Expr *post, Stmt *body, u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_FOR;
    s->line = line; s->column = col;
    s->for_stmt.init = init;
    s->for_stmt.cond = cond;
    s->for_stmt.post = post;
    s->for_stmt.body = body;
    return s;
}

internal Stmt *stmt_switch(Expr *expression, SwitchCase *cases, s32 num_cases, u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_SWITCH;
    s->line = line; s->column = col;
    s->switch_stmt.expr = expression;
    s->switch_stmt.cases = cases;
    s->switch_stmt.num_cases = num_cases;
    return s;
}

internal Stmt *stmt_break(u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_BREAK;
    s->line = line; s->column = col;
    return s;
}

internal Stmt *stmt_continue(u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_CONTINUE;
    s->line = line; s->column = col;
    return s;
}

internal Stmt *stmt_goto(String label, u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_GOTO;
    s->line = line; s->column = col;
    s->goto_stmt.label = label;
    return s;
}

internal Stmt *stmt_label(String label, Stmt *inner, u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_LABEL;
    s->line = line; s->column = col;
    s->label_stmt.label = label;
    s->label_stmt.stmt = inner;
    return s;
}

internal Stmt *stmt_block(Stmt **stmts, s32 num_stmts, u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_BLOCK;
    s->line = line; s->column = col;
    s->block.stmts = stmts;
    s->block.num_stmts = num_stmts;
    return s;
}

internal Stmt *stmt_decl(ASTDecl *declaration, u32 line, u32 col){
    Stmt *s = AST_ALLOC(Stmt);
    s->kind = STMT_DECL;
    s->line = line; s->column = col;
    s->decl.decl = declaration;
    return s;
}

/* ======================================================================
 * ASTDecl constructors
 * ====================================================================== */

internal ASTDecl *decl_var(TypeSpec *type, String name, Expr *init, StorageClass sc, u32 line, u32 col){
    ASTDecl *d = AST_ALLOC(ASTDecl);
    d->kind = DECL_VAR;
    d->line = line; d->column = col;
    d->storage = sc;
    d->var.type = type;
    d->var.name = name;
    d->var.init = init;
    return d;
}

internal ASTDecl *decl_func(TypeSpec *ret_type, String name, FuncParam *params, s32 num_params,
                            b32 has_varargs, Stmt *body, StorageClass sc, u32 line, u32 col){
    ASTDecl *d = AST_ALLOC(ASTDecl);
    d->kind = DECL_FUNC;
    d->line = line; d->column = col;
    d->storage = sc;
    d->func.ret_type = ret_type;
    d->func.name = name;
    d->func.params = params;
    d->func.num_params = num_params;
    d->func.has_varargs = has_varargs;
    d->func.body = body;
    return d;
}

internal ASTDecl *decl_struct(String name, AggregateField *fields, s32 num_fields, u32 line, u32 col){
    ASTDecl *d = AST_ALLOC(ASTDecl);
    d->kind = DECL_STRUCT;
    d->line = line; d->column = col;
    d->aggregate.name = name;
    d->aggregate.fields = fields;
    d->aggregate.num_fields = num_fields;
    return d;
}

internal ASTDecl *decl_union(String name, AggregateField *fields, s32 num_fields, u32 line, u32 col){
    ASTDecl *d = AST_ALLOC(ASTDecl);
    d->kind = DECL_UNION;
    d->line = line; d->column = col;
    d->aggregate.name = name;
    d->aggregate.fields = fields;
    d->aggregate.num_fields = num_fields;
    return d;
}

internal ASTDecl *decl_enum(String name, EnumItem *items, s32 num_items, u32 line, u32 col){
    ASTDecl *d = AST_ALLOC(ASTDecl);
    d->kind = DECL_ENUM;
    d->line = line; d->column = col;
    d->enumeration.name = name;
    d->enumeration.items = items;
    d->enumeration.num_items = num_items;
    return d;
}

internal ASTDecl *decl_typedef(TypeSpec *type, String name, u32 line, u32 col){
    ASTDecl *d = AST_ALLOC(ASTDecl);
    d->kind = DECL_TYPEDEF;
    d->line = line; d->column = col;
    d->type_def.type = type;
    d->type_def.name = name;
    return d;
}

/* ======================================================================
 * Pretty-printer
 * ====================================================================== */

internal void ast_print_typespec(TypeSpec *type){
    if(!type){ printf("(null-type)"); return; }
    switch(type->kind){
        case TYPESPEC_NAME:
            printf("%.*s", type->name.name.count, type->name.name.data);
            break;
        case TYPESPEC_PTR:
            printf("ptr(");
            ast_print_typespec(type->ptr.base);
            printf(")");
            break;
        case TYPESPEC_ARRAY:
            printf("array(");
            ast_print_typespec(type->array.elem);
            printf(", ");
            if(type->array.size) ast_print_expr(type->array.size, 0);
            else printf("?");
            printf(")");
            break;
        case TYPESPEC_FUNC:
            printf("func(");
            ast_print_typespec(type->func.ret);
            printf(", (");
            for(s32 i = 0; i < (s32)darr_len(type->func.params); ++i){
                if(i > 0) printf(", ");
                ast_print_typespec(type->func.params[i]);
            }
            if(type->func.has_varargs) printf(", ...");
            printf("))");
            break;
        case TYPESPEC_CONST:
            printf("const ");
            ast_print_typespec(type->ptr.base);
            break;
        case TYPESPEC_VOLATILE:
            printf("volatile ");
            ast_print_typespec(type->ptr.base);
            break;
        case TYPESPEC_STRUCT:
            printf("struct %.*s", type->aggregate.name.count, type->aggregate.name.data);
            break;
        case TYPESPEC_UNION:
            printf("union %.*s", type->aggregate.name.count, type->aggregate.name.data);
            break;
        case TYPESPEC_ENUM:
            printf("enum %.*s", type->enumeration.name.count, type->enumeration.name.data);
            break;
        default:
            printf("(unknown-type)");
            break;
    }
}

internal void ast_print_expr(Expr *expr, s32 indent){
    if(!expr){ ast_indent(indent); printf("(null)\n"); return; }
    ast_indent(indent);
    switch(expr->kind){
        case EXPR_INT:
            printf("INT(%llu)\n", (unsigned long long)expr->int_lit.val);
            break;
        case EXPR_FLOAT:
            printf("FLOAT(%f)\n", expr->float_lit.val);
            break;
        case EXPR_STR:
            printf("STR(\"%.*s\")\n", expr->str_lit.val.count, expr->str_lit.val.data);
            break;
        case EXPR_NAME:
            printf("NAME(%.*s)\n", expr->name.name.count, expr->name.name.data);
            break;
        case EXPR_UNARY:
            printf("UNARY(op=%d)\n", expr->unary.op);
            ast_print_expr(expr->unary.operand, indent + 1);
            break;
        case EXPR_POSTFIX:
            printf("POSTFIX(op=%d)\n", expr->unary.op);
            ast_print_expr(expr->unary.operand, indent + 1);
            break;
        case EXPR_BINARY:
            printf("BINARY(op=%d)\n", expr->binary.op);
            ast_print_expr(expr->binary.left, indent + 1);
            ast_print_expr(expr->binary.right, indent + 1);
            break;
        case EXPR_TERNARY:
            printf("TERNARY\n");
            ast_print_expr(expr->ternary.cond, indent + 1);
            ast_print_expr(expr->ternary.then_expr, indent + 1);
            ast_print_expr(expr->ternary.else_expr, indent + 1);
            break;
        case EXPR_CALL:
            printf("CALL(num_args=%d)\n", expr->call.num_args);
            ast_print_expr(expr->call.func, indent + 1);
            for(s32 i = 0; i < expr->call.num_args; ++i)
                ast_print_expr(expr->call.args[i], indent + 1);
            break;
        case EXPR_INDEX:
            printf("INDEX\n");
            ast_print_expr(expr->index.array, indent + 1);
            ast_print_expr(expr->index.index, indent + 1);
            break;
        case EXPR_FIELD:
            printf("FIELD(.%.*s)\n", expr->field.field_name.count, expr->field.field_name.data);
            ast_print_expr(expr->field.object, indent + 1);
            break;
        case EXPR_FIELD_PTR:
            printf("FIELD_PTR(->%.*s)\n", expr->field.field_name.count, expr->field.field_name.data);
            ast_print_expr(expr->field.object, indent + 1);
            break;
        case EXPR_CAST:
            printf("CAST(");
            ast_print_typespec(expr->cast.type);
            printf(")\n");
            ast_print_expr(expr->cast.expr, indent + 1);
            break;
        case EXPR_SIZEOF_EXPR:
            printf("SIZEOF_EXPR\n");
            ast_print_expr(expr->sizeof_expr.expr, indent + 1);
            break;
        case EXPR_SIZEOF_TYPE:
            printf("SIZEOF_TYPE(");
            ast_print_typespec(expr->sizeof_type.type);
            printf(")\n");
            break;
        case EXPR_PAREN:
            printf("PAREN\n");
            ast_print_expr(expr->paren.inner, indent + 1);
            break;
        default:
            printf("(unknown-expr)\n");
            break;
    }
}

internal void ast_print_stmt(Stmt *stmt, s32 indent){
    if(!stmt){ ast_indent(indent); printf("(null-stmt)\n"); return; }
    ast_indent(indent);
    switch(stmt->kind){
        case STMT_EXPR:
            printf("EXPR_STMT\n");
            ast_print_expr(stmt->expr.expr, indent + 1);
            break;
        case STMT_RETURN:
            printf("RETURN\n");
            if(stmt->ret.expr) ast_print_expr(stmt->ret.expr, indent + 1);
            break;
        case STMT_IF:
            printf("IF\n");
            ast_indent(indent + 1); printf("cond:\n");
            ast_print_expr(stmt->if_stmt.cond, indent + 2);
            ast_indent(indent + 1); printf("then:\n");
            ast_print_stmt(stmt->if_stmt.then_body, indent + 2);
            if(stmt->if_stmt.else_body){
                ast_indent(indent + 1); printf("else:\n");
                ast_print_stmt(stmt->if_stmt.else_body, indent + 2);
            }
            break;
        case STMT_WHILE:
            printf("WHILE\n");
            ast_print_expr(stmt->while_stmt.cond, indent + 1);
            ast_print_stmt(stmt->while_stmt.body, indent + 1);
            break;
        case STMT_DO_WHILE:
            printf("DO_WHILE\n");
            ast_print_stmt(stmt->do_while_stmt.body, indent + 1);
            ast_print_expr(stmt->do_while_stmt.cond, indent + 1);
            break;
        case STMT_FOR:
            printf("FOR\n");
            if(stmt->for_stmt.init) ast_print_stmt(stmt->for_stmt.init, indent + 1);
            if(stmt->for_stmt.cond) ast_print_expr(stmt->for_stmt.cond, indent + 1);
            if(stmt->for_stmt.post) ast_print_expr(stmt->for_stmt.post, indent + 1);
            ast_print_stmt(stmt->for_stmt.body, indent + 1);
            break;
        case STMT_SWITCH:
            printf("SWITCH\n");
            ast_print_expr(stmt->switch_stmt.expr, indent + 1);
            for(s32 i = 0; i < stmt->switch_stmt.num_cases; ++i){
                SwitchCase *c = &stmt->switch_stmt.cases[i];
                ast_indent(indent + 1);
                if(c->is_default) printf("DEFAULT:\n");
                else{ printf("CASE:\n"); ast_print_expr(c->value, indent + 2); }
                for(s32 j = 0; j < c->num_stmts; ++j)
                    ast_print_stmt(c->stmts[j], indent + 2);
            }
            break;
        case STMT_BREAK:
            printf("BREAK\n");
            break;
        case STMT_CONTINUE:
            printf("CONTINUE\n");
            break;
        case STMT_GOTO:
            printf("GOTO(%.*s)\n", stmt->goto_stmt.label.count, stmt->goto_stmt.label.data);
            break;
        case STMT_LABEL:
            printf("LABEL(%.*s)\n", stmt->label_stmt.label.count, stmt->label_stmt.label.data);
            ast_print_stmt(stmt->label_stmt.stmt, indent + 1);
            break;
        case STMT_BLOCK:
            printf("BLOCK(%d stmts)\n", stmt->block.num_stmts);
            for(s32 i = 0; i < stmt->block.num_stmts; ++i)
                ast_print_stmt(stmt->block.stmts[i], indent + 1);
            break;
        case STMT_DECL:
            printf("DECL_STMT\n");
            ast_print_decl(stmt->decl.decl, indent + 1);
            break;
        default:
            printf("(unknown-stmt)\n");
            break;
    }
}

internal void ast_print_decl(ASTDecl *decl, s32 indent){
    if(!decl){ ast_indent(indent); printf("(null-decl)\n"); return; }
    ast_indent(indent);
    switch(decl->kind){
        case DECL_VAR:
            printf("VAR(%.*s : ", decl->var.name.count, decl->var.name.data);
            ast_print_typespec(decl->var.type);
            printf(")\n");
            if(decl->var.init) ast_print_expr(decl->var.init, indent + 1);
            break;
        case DECL_FUNC:
            printf("FUNC(%.*s : ", decl->func.name.count, decl->func.name.data);
            ast_print_typespec(decl->func.ret_type);
            printf(", %d params)\n", decl->func.num_params);
            for(s32 i = 0; i < decl->func.num_params; ++i){
                ast_indent(indent + 1);
                printf("PARAM(%.*s : ", decl->func.params[i].name.count,
                       decl->func.params[i].name.data);
                ast_print_typespec(decl->func.params[i].type);
                printf(")\n");
            }
            if(decl->func.body) ast_print_stmt(decl->func.body, indent + 1);
            break;
        case DECL_STRUCT:
            printf("STRUCT(%.*s, %d fields)\n", decl->aggregate.name.count,
                   decl->aggregate.name.data, decl->aggregate.num_fields);
            for(s32 i = 0; i < decl->aggregate.num_fields; ++i){
                ast_indent(indent + 1);
                printf("FIELD(%.*s : ", decl->aggregate.fields[i].name.count,
                       decl->aggregate.fields[i].name.data);
                ast_print_typespec(decl->aggregate.fields[i].type);
                printf(")\n");
            }
            break;
        case DECL_UNION:
            printf("UNION(%.*s, %d fields)\n", decl->aggregate.name.count,
                   decl->aggregate.name.data, decl->aggregate.num_fields);
            for(s32 i = 0; i < decl->aggregate.num_fields; ++i){
                ast_indent(indent + 1);
                printf("FIELD(%.*s : ", decl->aggregate.fields[i].name.count,
                       decl->aggregate.fields[i].name.data);
                ast_print_typespec(decl->aggregate.fields[i].type);
                printf(")\n");
            }
            break;
        case DECL_ENUM:
            printf("ENUM(%.*s, %d items)\n", decl->enumeration.name.count,
                   decl->enumeration.name.data, decl->enumeration.num_items);
            for(s32 i = 0; i < decl->enumeration.num_items; ++i){
                ast_indent(indent + 1);
                printf("ITEM(%.*s", decl->enumeration.items[i].name.count,
                       decl->enumeration.items[i].name.data);
                if(decl->enumeration.items[i].value){
                    printf(" = ");
                    ast_print_expr(decl->enumeration.items[i].value, 0);
                }
                else printf(")\n");
            }
            break;
        case DECL_TYPEDEF:
            printf("TYPEDEF(%.*s = ", decl->type_def.name.count, decl->type_def.name.data);
            ast_print_typespec(decl->type_def.type);
            printf(")\n");
            break;
        default:
            printf("(unknown-decl)\n");
            break;
    }
}
