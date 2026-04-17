
/*
 * eval.h — Tree-walking interpreter for C99.
 *
 * Core concepts:
 *   Val        — runtime value (tagged union: int, float, pointer, struct, array, etc.)
 *   Env        — environment: a scope chain mapping names to Vals
 *   Interp     — interpreter state holding the global environment and function table
 *
 * Control flow is handled via setjmp/longjmp for return, break, continue, goto.
 */

#if !defined(EVAL_H)
#define EVAL_H

#include <stdarg.h>
#include <setjmp.h>

/* ======================================================================
 * Runtime value representation
 * ====================================================================== */

typedef enum ValKind {
    VAL_VOID = 0,
    VAL_INT,          /* s64 — covers char, short, int, long, long long    */
    VAL_FLOAT,        /* f64 — covers float, double                        */
    VAL_PTR,          /* pointer to a heap-allocated Val                    */
    VAL_ARRAY,        /* contiguous block of Vals                           */
    VAL_STRUCT,       /* named fields                                       */
    VAL_FUNC,         /* reference to an ASTDecl function                   */
    VAL_STRING,       /* C string (char *)                                  */
} ValKind;

typedef struct Val Val;
typedef struct StructField StructField;

struct StructField {
    String name;
    Val *value;
};

struct Val {
    ValKind kind;
    union {
        s64 i;                              /* VAL_INT */
        f64 f;                              /* VAL_FLOAT */
        Val *ptr;                           /* VAL_PTR — points to another Val */
        struct { Val *elems; s32 len; } arr; /* VAL_ARRAY */
        struct {
            String name;
            StructField *fields;            /* plain array */
            s32 num_fields;
        } strct;                            /* VAL_STRUCT */
        ASTDecl *func;                      /* VAL_FUNC — function AST node */
        char *str;                          /* VAL_STRING */
    };
};

/* ======================================================================
 * Environment — scope chain
 *
 * Each scope is a linked list node holding a dynamic array of bindings.
 * Lookup walks from innermost to outermost scope.
 * ====================================================================== */

typedef struct Binding {
    String name;
    Val val;
} Binding;

typedef struct Scope Scope;
struct Scope {
    Binding *bindings;       /* DynamicArray */
    Scope *parent;
};

/* ======================================================================
 * Control flow signals
 *
 * Used with longjmp to unwind the call stack for return/break/continue.
 * ====================================================================== */

typedef enum FlowKind {
    FLOW_NONE = 0,
    FLOW_RETURN,
    FLOW_BREAK,
    FLOW_CONTINUE,
    FLOW_GOTO,
} FlowKind;

/* ======================================================================
 * Interpreter state
 * ====================================================================== */

typedef struct Interp {
    Scope *global_scope;
    Scope *current_scope;
    Val return_val;
    FlowKind flow;
    String goto_label;
    b32 had_error;
} Interp;

/* ======================================================================
 * Builtin function type
 * ====================================================================== */

typedef Val (*BuiltinFn)(Interp *interp, Val *args, s32 num_args);

typedef struct BuiltinEntry {
    const char *name;
    BuiltinFn fn;
} BuiltinEntry;

/* ======================================================================
 * Public API
 * ====================================================================== */

internal Interp interp_init(void);
internal void interp_run(Interp *interp, ASTDecl **decls);

internal Scope *scope_push(Interp *interp);
internal void scope_pop(Interp *interp);
internal void scope_define(Scope *scope, String name, Val val);
internal Val *scope_lookup(Interp *interp, String name);
internal Val *scope_lookup_local(Scope *scope, String name);

internal Val eval_expr(Interp *interp, Expr *expr);
internal Val *eval_lvalue(Interp *interp, Expr *expr);
internal void exec_stmt(Interp *interp, Stmt *stmt);
internal void exec_decl(Interp *interp, ASTDecl *decl);

internal Val call_function(Interp *interp, ASTDecl *func, Val *args, s32 num_args);

internal Val val_void(void);
internal Val val_int(s64 v);
internal Val val_float(f64 v);
internal Val val_ptr(Val *p);
internal Val val_string(char *s);
internal Val val_func(ASTDecl *decl);
internal s64 val_to_int(Val v);
internal f64 val_to_float(Val v);
internal b32 val_is_truthy(Val v);

internal s64 type_sizeof(TypeSpec *type);

/* Builtins */
internal Val builtin_printf(Interp *interp, Val *args, s32 num_args);
internal Val builtin_putchar(Interp *interp, Val *args, s32 num_args);
internal Val builtin_puts(Interp *interp, Val *args, s32 num_args);
internal Val builtin_malloc(Interp *interp, Val *args, s32 num_args);
internal Val builtin_free(Interp *interp, Val *args, s32 num_args);
internal Val builtin_exit(Interp *interp, Val *args, s32 num_args);
internal Val builtin_abs(Interp *interp, Val *args, s32 num_args);

#endif /* EVAL_H */
