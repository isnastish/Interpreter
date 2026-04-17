
/*
 * ast.h — Abstract Syntax Tree node definitions for a C99 parser.
 *
 * Four core node families:
 *   TypeSpec  — type representations (int, struct, pointer, array, function type)
 *   Expr      — expressions (literals, binary/unary ops, calls, casts, etc.)
 *   Stmt      — statements (if, while, for, return, compound blocks, etc.)
 *   Decl      — declarations (variables, functions, structs, unions, enums, typedefs)
 *
 * All nodes are heap-allocated and accessed through pointers.
 * List-bearing fields use the DynamicArray macros from common.h.
 */

#if !defined(AST_H)
#define AST_H

/* ======================================================================
 * Forward declarations
 * ====================================================================== */

typedef struct TypeSpec TypeSpec;
typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct ASTDecl ASTDecl;

typedef struct AggregateField AggregateField;
typedef struct EnumItem EnumItem;
typedef struct FuncParam FuncParam;
typedef struct SwitchCase SwitchCase;

/* ======================================================================
 * TypeSpec — C99 type representations
 *
 * Examples:
 *   int           -> TYPESPEC_NAME("int")
 *   const int     -> TYPESPEC_CONST(TYPESPEC_NAME("int"))
 *   int *         -> TYPESPEC_PTR(TYPESPEC_NAME("int"))
 *   int[10]       -> TYPESPEC_ARRAY(TYPESPEC_NAME("int"), size=10)
 *   int(*)(int)   -> TYPESPEC_PTR(TYPESPEC_FUNC(...))
 *   struct S{..}  -> TYPESPEC_STRUCT
 *   enum E{..}    -> TYPESPEC_ENUM
 * ====================================================================== */

typedef enum TypeSpecKind {
    TYPESPEC_NONE = 0,
    TYPESPEC_NAME,        /* named type: "int", "char", "MyType"            */
    TYPESPEC_PTR,         /* pointer: int *                                 */
    TYPESPEC_ARRAY,       /* array: int[10] or int[]                        */
    TYPESPEC_FUNC,        /* function type: int(int, int)                   */
    TYPESPEC_CONST,       /* const qualifier wrapper                        */
    TYPESPEC_VOLATILE,    /* volatile qualifier wrapper                     */
    TYPESPEC_STRUCT,      /* struct { fields }                              */
    TYPESPEC_UNION,       /* union { fields }                               */
    TYPESPEC_ENUM,        /* enum { items }                                 */
} TypeSpecKind;

struct TypeSpec {
    TypeSpecKind kind;
    union {
        /* TYPESPEC_NAME */
        struct { String name; } name;

        /* TYPESPEC_PTR, TYPESPEC_CONST, TYPESPEC_VOLATILE */
        struct { TypeSpec *base; } ptr;

        /* TYPESPEC_ARRAY */
        struct { TypeSpec *elem; Expr *size; } array;

        /* TYPESPEC_FUNC: return_type + param types */
        struct {
            TypeSpec *ret;
            TypeSpec **params;   /* DynamicArray of TypeSpec* */
            b32 has_varargs;     /* 1 if last param is ... */
        } func;

        /* TYPESPEC_STRUCT, TYPESPEC_UNION */
        struct {
            String name;               /* may be empty for anonymous */
            AggregateField *fields;    /* DynamicArray */
            s32 num_fields;
        } aggregate;

        /* TYPESPEC_ENUM */
        struct {
            String name;
            EnumItem *items;           /* DynamicArray */
            s32 num_items;
        } enumeration;
    };
};

/* ======================================================================
 * AggregateField — struct/union member
 *   Example: "int x;" or "struct Inner nested;"
 * ====================================================================== */

struct AggregateField {
    TypeSpec *type;
    String name;
    Expr *bitfield;   /* NULL if not a bitfield */
};

/* ======================================================================
 * EnumItem — enumerator constant
 *   Example: "TOKEN_PLUS = 43" or "TOKEN_EOF"
 * ====================================================================== */

struct EnumItem {
    String name;
    Expr *value;      /* NULL if implicit */
};

/* ======================================================================
 * FuncParam — function parameter
 *   Example: "int argc" or "const char *argv"
 * ====================================================================== */

struct FuncParam {
    TypeSpec *type;
    String name;      /* may be empty for abstract declarators */
};

/* ======================================================================
 * Expr — C99 expressions
 *
 * Every expression node has a kind tag. The union holds kind-specific data.
 * ====================================================================== */

typedef enum ExprKind {
    EXPR_NONE = 0,
    EXPR_INT,         /* integer literal: 42, 0xFF, 'a'                   */
    EXPR_FLOAT,       /* floating literal: 3.14, 1e-5                     */
    EXPR_STR,         /* string literal: "hello"                          */
    EXPR_NAME,        /* identifier: x, printf                            */
    EXPR_UNARY,       /* prefix unary: -x, !x, *p, &x, ++x, --x         */
    EXPR_POSTFIX,     /* postfix unary: x++, x--                          */
    EXPR_BINARY,      /* binary: a + b, a << b, a && b                    */
    EXPR_TERNARY,     /* conditional: cond ? then_expr : else_expr        */
    EXPR_CALL,        /* function call: f(a, b)                           */
    EXPR_INDEX,       /* array subscript: a[i]                            */
    EXPR_FIELD,       /* member access: s.x                               */
    EXPR_FIELD_PTR,   /* pointer member access: p->x                      */
    EXPR_CAST,        /* type cast: (int)x                                */
    EXPR_SIZEOF_EXPR, /* sizeof applied to expression: sizeof x           */
    EXPR_SIZEOF_TYPE, /* sizeof applied to type: sizeof(int)              */
    EXPR_COMPOUND,    /* compound literal: (int[]){1,2,3}                 */
    EXPR_PAREN,       /* parenthesised expression: (x + y)                */
} ExprKind;

struct Expr {
    ExprKind kind;
    u32 line;
    u32 column;
    union {
        /* EXPR_INT */
        struct { u64 val; } int_lit;

        /* EXPR_FLOAT */
        struct { f64 val; } float_lit;

        /* EXPR_STR */
        struct { String val; } str_lit;

        /* EXPR_NAME */
        struct { String name; } name;

        /* EXPR_UNARY, EXPR_POSTFIX */
        struct { TokenKind op; Expr *operand; } unary;

        /* EXPR_BINARY */
        struct { TokenKind op; Expr *left; Expr *right; } binary;

        /* EXPR_TERNARY */
        struct { Expr *cond; Expr *then_expr; Expr *else_expr; } ternary;

        /* EXPR_CALL */
        struct { Expr *func; Expr **args; s32 num_args; } call;

        /* EXPR_INDEX */
        struct { Expr *array; Expr *index; } index;

        /* EXPR_FIELD, EXPR_FIELD_PTR */
        struct { Expr *object; String field_name; } field;

        /* EXPR_CAST */
        struct { TypeSpec *type; Expr *expr; } cast;

        /* EXPR_SIZEOF_EXPR */
        struct { Expr *expr; } sizeof_expr;

        /* EXPR_SIZEOF_TYPE */
        struct { TypeSpec *type; } sizeof_type;

        /* EXPR_COMPOUND */
        struct { TypeSpec *type; Expr **elems; s32 num_elems; } compound;

        /* EXPR_PAREN */
        struct { Expr *inner; } paren;
    };
};

/* ======================================================================
 * SwitchCase — a single case or default arm in a switch
 * ====================================================================== */

struct SwitchCase {
    Expr *value;       /* NULL for 'default:' */
    Stmt **stmts;      /* DynamicArray */
    s32 num_stmts;
    b32 is_default;
};

/* ======================================================================
 * Stmt — C99 statements
 * ====================================================================== */

typedef enum StmtKind {
    STMT_NONE = 0,
    STMT_EXPR,        /* expression statement: x = 5;                     */
    STMT_RETURN,      /* return statement: return expr;                   */
    STMT_IF,          /* if / else if / else                              */
    STMT_WHILE,       /* while (cond) body                                */
    STMT_DO_WHILE,    /* do body while (cond);                            */
    STMT_FOR,         /* for (init; cond; post) body                      */
    STMT_SWITCH,      /* switch (expr) { cases }                          */
    STMT_BREAK,       /* break;                                           */
    STMT_CONTINUE,    /* continue;                                        */
    STMT_GOTO,        /* goto label;                                      */
    STMT_LABEL,       /* label: stmt                                      */
    STMT_BLOCK,       /* compound statement: { stmts }                    */
    STMT_DECL,        /* declaration used as statement inside a block     */
} StmtKind;

struct Stmt {
    StmtKind kind;
    u32 line;
    u32 column;
    union {
        /* STMT_EXPR */
        struct { Expr *expr; } expr;

        /* STMT_RETURN */
        struct { Expr *expr; /* may be NULL */ } ret;

        /* STMT_IF */
        struct {
            Expr *cond;
            Stmt *then_body;
            Stmt *else_body;     /* NULL if no else */
        } if_stmt;

        /* STMT_WHILE */
        struct { Expr *cond; Stmt *body; } while_stmt;

        /* STMT_DO_WHILE */
        struct { Stmt *body; Expr *cond; } do_while_stmt;

        /* STMT_FOR */
        struct {
            Stmt *init;          /* may be expr-stmt or decl-stmt, or NULL */
            Expr *cond;          /* may be NULL */
            Expr *post;          /* may be NULL */
            Stmt *body;
        } for_stmt;

        /* STMT_SWITCH */
        struct {
            Expr *expr;
            SwitchCase *cases;   /* DynamicArray */
            s32 num_cases;
        } switch_stmt;

        /* STMT_GOTO */
        struct { String label; } goto_stmt;

        /* STMT_LABEL */
        struct { String label; Stmt *stmt; } label_stmt;

        /* STMT_BREAK, STMT_CONTINUE — no extra data */

        /* STMT_BLOCK */
        struct { Stmt **stmts; s32 num_stmts; } block;

        /* STMT_DECL */
        struct { ASTDecl *decl; } decl;
    };
};

/* ======================================================================
 * ASTDecl — C99 declarations
 *
 * Renamed from 'Decl' to avoid collision with the original parser.h Decl.
 * ====================================================================== */

typedef enum ASTDeclKind {
    DECL_NONE = 0,
    DECL_VAR,         /* variable declaration: int x = 5;                 */
    DECL_FUNC,        /* function definition/declaration                  */
    DECL_STRUCT,      /* struct Foo { ... };                              */
    DECL_UNION,       /* union Bar { ... };                               */
    DECL_ENUM,        /* enum Color { ... };                              */
    DECL_TYPEDEF,     /* typedef unsigned int uint;                       */
} ASTDeclKind;

/* Storage class specifiers */
typedef enum StorageClass {
    STORAGE_NONE = 0,
    STORAGE_STATIC,
    STORAGE_EXTERN,
    STORAGE_AUTO,
    STORAGE_REGISTER,
} StorageClass;

struct ASTDecl {
    ASTDeclKind kind;
    u32 line;
    u32 column;
    StorageClass storage;
    union {
        /* DECL_VAR */
        struct {
            TypeSpec *type;
            String name;
            Expr *init;          /* NULL if no initializer */
        } var;

        /* DECL_FUNC */
        struct {
            TypeSpec *ret_type;
            String name;
            FuncParam *params;   /* DynamicArray */
            s32 num_params;
            b32 has_varargs;
            Stmt *body;          /* NULL for forward declarations */
        } func;

        /* DECL_STRUCT, DECL_UNION */
        struct {
            String name;               /* may be empty for anonymous */
            AggregateField *fields;    /* DynamicArray */
            s32 num_fields;
        } aggregate;

        /* DECL_ENUM */
        struct {
            String name;
            EnumItem *items;           /* DynamicArray */
            s32 num_items;
        } enumeration;

        /* DECL_TYPEDEF */
        struct {
            TypeSpec *type;
            String name;
        } type_def;
    };
};

/* ======================================================================
 * AST constructor prototypes (implemented in ast.c)
 * ====================================================================== */

/* --- TypeSpec constructors --- */
internal TypeSpec *typespec_name(String name);
internal TypeSpec *typespec_ptr(TypeSpec *base);
internal TypeSpec *typespec_array(TypeSpec *elem, Expr *size);
internal TypeSpec *typespec_func(TypeSpec *ret, TypeSpec **params, b32 has_varargs);
internal TypeSpec *typespec_const(TypeSpec *base);
internal TypeSpec *typespec_volatile(TypeSpec *base);

/* --- Expr constructors --- */
internal Expr *expr_int(u64 val, u32 line, u32 col);
internal Expr *expr_float(f64 val, u32 line, u32 col);
internal Expr *expr_str(String val, u32 line, u32 col);
internal Expr *expr_name(String name, u32 line, u32 col);
internal Expr *expr_unary(TokenKind op, Expr *operand, u32 line, u32 col);
internal Expr *expr_postfix(TokenKind op, Expr *operand, u32 line, u32 col);
internal Expr *expr_binary(TokenKind op, Expr *left, Expr *right, u32 line, u32 col);
internal Expr *expr_ternary(Expr *cond, Expr *then_expr, Expr *else_expr, u32 line, u32 col);
internal Expr *expr_call(Expr *func, Expr **args, s32 num_args, u32 line, u32 col);
internal Expr *expr_index(Expr *array, Expr *index, u32 line, u32 col);
internal Expr *expr_field(Expr *object, String field_name, u32 line, u32 col);
internal Expr *expr_field_ptr(Expr *object, String field_name, u32 line, u32 col);
internal Expr *expr_cast(TypeSpec *type, Expr *expr, u32 line, u32 col);
internal Expr *expr_sizeof_expr(Expr *expr, u32 line, u32 col);
internal Expr *expr_sizeof_type(TypeSpec *type, u32 line, u32 col);
internal Expr *expr_paren(Expr *inner, u32 line, u32 col);

/* --- Stmt constructors --- */
internal Stmt *stmt_expr(Expr *expr, u32 line, u32 col);
internal Stmt *stmt_return(Expr *expr, u32 line, u32 col);
internal Stmt *stmt_if(Expr *cond, Stmt *then_body, Stmt *else_body, u32 line, u32 col);
internal Stmt *stmt_while(Expr *cond, Stmt *body, u32 line, u32 col);
internal Stmt *stmt_do_while(Stmt *body, Expr *cond, u32 line, u32 col);
internal Stmt *stmt_for(Stmt *init, Expr *cond, Expr *post, Stmt *body, u32 line, u32 col);
internal Stmt *stmt_switch(Expr *expr, SwitchCase *cases, s32 num_cases, u32 line, u32 col);
internal Stmt *stmt_break(u32 line, u32 col);
internal Stmt *stmt_continue(u32 line, u32 col);
internal Stmt *stmt_goto(String label, u32 line, u32 col);
internal Stmt *stmt_label(String label, Stmt *stmt, u32 line, u32 col);
internal Stmt *stmt_block(Stmt **stmts, s32 num_stmts, u32 line, u32 col);
internal Stmt *stmt_decl(ASTDecl *decl, u32 line, u32 col);

/* --- Decl constructors --- */
internal ASTDecl *decl_var(TypeSpec *type, String name, Expr *init, StorageClass sc, u32 line, u32 col);
internal ASTDecl *decl_func(TypeSpec *ret_type, String name, FuncParam *params, s32 num_params,
                            b32 has_varargs, Stmt *body, StorageClass sc, u32 line, u32 col);
internal ASTDecl *decl_struct(String name, AggregateField *fields, s32 num_fields, u32 line, u32 col);
internal ASTDecl *decl_union(String name, AggregateField *fields, s32 num_fields, u32 line, u32 col);
internal ASTDecl *decl_enum(String name, EnumItem *items, s32 num_items, u32 line, u32 col);
internal ASTDecl *decl_typedef(TypeSpec *type, String name, u32 line, u32 col);

/* --- Pretty-printer --- */
internal void ast_print_expr(Expr *expr, s32 indent);
internal void ast_print_stmt(Stmt *stmt, s32 indent);
internal void ast_print_decl(ASTDecl *decl, s32 indent);
internal void ast_print_typespec(TypeSpec *type);

#endif /* AST_H */
