
/*
 * parser.h — Recursive-descent parser for C99.
 *
 * Grammar (simplified EBNF notation):
 *
 * ========== Top-level ==========
 *
 * translation_unit := decl*
 *
 * ========== Declarations ==========
 *
 * decl := func_decl | var_decl | struct_decl | union_decl | enum_decl | typedef_decl
 *
 * storage_class := 'static' | 'extern' | 'auto' | 'register'
 *
 * type_qualifier := 'const' | 'volatile'
 *
 * base_type := 'void' | 'char' | 'short' | 'int' | 'long' | 'float' | 'double'
 *            | 'signed' | 'unsigned'
 *            | 'struct' name? '{' struct_field* '}'
 *            | 'struct' name
 *            | 'union' name? '{' struct_field* '}'
 *            | 'union' name
 *            | 'enum' name? '{' enum_item (',' enum_item)* ','? '}'
 *            | 'enum' name
 *            | name   (typedef'd name)
 *
 * type_specifier := type_qualifier* base_type+ type_qualifier* pointer*
 *
 * pointer := '*' type_qualifier*
 *
 * struct_field := type_specifier name (':' const_expr)? ';'
 *
 * enum_item := name ('=' const_expr)?
 *
 * var_decl := storage_class? type_specifier name ('=' initializer)? ';'
 *
 * func_param := type_specifier name?
 * func_param_list := func_param (',' func_param)* (',' '...')?
 *                  | 'void'
 *                  | '...'
 *
 * func_decl := storage_class? type_specifier name '(' func_param_list? ')' (compound_stmt | ';')
 *
 * typedef_decl := 'typedef' type_specifier name ';'
 *
 * struct_decl := 'struct' name? '{' struct_field* '}' ';'
 * union_decl  := 'union'  name? '{' struct_field* '}' ';'
 * enum_decl   := 'enum'   name? '{' enum_item (',' enum_item)* ','? '}' ';'
 *
 * ========== Statements ==========
 *
 * stmt := compound_stmt | if_stmt | while_stmt | do_while_stmt | for_stmt
 *       | switch_stmt | return_stmt | break_stmt | continue_stmt | goto_stmt
 *       | label_stmt | expr_stmt | decl_stmt
 *
 * compound_stmt := '{' (stmt | decl)* '}'
 * if_stmt       := 'if' '(' expr ')' stmt ('else' stmt)?
 * while_stmt    := 'while' '(' expr ')' stmt
 * do_while_stmt := 'do' stmt 'while' '(' expr ')' ';'
 * for_stmt      := 'for' '(' (expr | decl)? ';' expr? ';' expr? ')' stmt
 * switch_stmt   := 'switch' '(' expr ')' '{' switch_case* '}'
 * switch_case   := ('case' const_expr | 'default') ':' stmt*
 * return_stmt   := 'return' expr? ';'
 * break_stmt    := 'break' ';'
 * continue_stmt := 'continue' ';'
 * goto_stmt     := 'goto' name ';'
 * label_stmt    := name ':' stmt
 * expr_stmt     := expr? ';'
 *
 * ========== Expressions (C99 precedence, lowest to highest) ==========
 *
 * expr           := assign (',' assign)*
 * assign         := ternary (assign_op assign)?
 * assign_op      := '=' | '+=' | '-=' | '*=' | '/=' | '%=' | '&=' | '|='
 *                 | '^=' | '<<=' | '>>='
 * ternary        := logical_or ('?' expr ':' ternary)?
 * logical_or     := logical_and ('||' logical_and)*
 * logical_and    := bitwise_or ('&&' bitwise_or)*
 * bitwise_or     := bitwise_xor ('|' bitwise_xor)*
 * bitwise_xor    := bitwise_and ('^' bitwise_and)*
 * bitwise_and    := equality ('&' equality)*
 * equality       := relational (('==' | '!=') relational)*
 * relational     := shift (('<' | '>' | '<=' | '>=') shift)*
 * shift          := additive (('<<' | '>>') additive)*
 * additive       := multiplicative (('+' | '-') multiplicative)*
 * multiplicative := cast (('*' | '/' | '%') cast)*
 * cast           := '(' type_name ')' cast | unary
 * unary          := ('++' | '--') unary | unary_op cast
 *                 | 'sizeof' unary | 'sizeof' '(' type_name ')'
 *                 | postfix
 * unary_op       := '&' | '*' | '+' | '-' | '~' | '!'
 * postfix        := primary (postfix_suffix)*
 * postfix_suffix := '(' arg_list? ')' | '[' expr ']' | '.' name
 *                 | '->' name | '++' | '--'
 * primary        := int_literal | float_literal | string_literal
 *                 | char_literal | name | '(' expr ')'
 */

#if !defined(PARSER_H)
#define PARSER_H

/* ======================================================================
 * Parser state
 * ====================================================================== */

typedef struct Parser {
    Lexer *lexer;
    b32 had_error;
    b32 panic_mode;
} Parser;

internal Parser parser_init(Lexer *lexer);

/* ======================================================================
 * Parser error reporting
 * ====================================================================== */

internal void parse_error(Parser *p, const char *msg);
internal void parse_error_at(Parser *p, u32 line, u32 col, const char *msg);

/* ======================================================================
 * Token helpers
 * ====================================================================== */

internal void parser_advance(Parser *p);
internal b32 parser_check(Parser *p, s32 kind);
internal b32 parser_match(Parser *p, s32 kind);
internal void parser_expect(Parser *p, s32 kind, const char *msg);

/* ======================================================================
 * Type parsing
 * ====================================================================== */

internal b32 is_type_token(Parser *p);
internal TypeSpec *parse_base_type(Parser *p);
internal TypeSpec *parse_full_type(Parser *p);

/* ======================================================================
 * Expression parsing (C99 precedence)
 * ====================================================================== */

internal Expr *parse_expr(Parser *p);
internal Expr *parse_expr_assign(Parser *p);
internal Expr *parse_expr_ternary(Parser *p);
internal Expr *parse_expr_logical_or(Parser *p);
internal Expr *parse_expr_logical_and(Parser *p);
internal Expr *parse_expr_bitwise_or(Parser *p);
internal Expr *parse_expr_bitwise_xor(Parser *p);
internal Expr *parse_expr_bitwise_and(Parser *p);
internal Expr *parse_expr_equality(Parser *p);
internal Expr *parse_expr_relational(Parser *p);
internal Expr *parse_expr_shift(Parser *p);
internal Expr *parse_expr_additive(Parser *p);
internal Expr *parse_expr_multiplicative(Parser *p);
internal Expr *parse_expr_cast(Parser *p);
internal Expr *parse_expr_unary(Parser *p);
internal Expr *parse_expr_postfix(Parser *p);
internal Expr *parse_expr_primary(Parser *p);

/* ======================================================================
 * Statement parsing
 * ====================================================================== */

internal Stmt *parse_stmt(Parser *p);
internal Stmt *parse_stmt_block(Parser *p);

/* ======================================================================
 * Declaration parsing
 * ====================================================================== */

internal ASTDecl *parse_decl(Parser *p);
internal ASTDecl *parse_decl_enum(Parser *p);
internal ASTDecl *parse_decl_struct(Parser *p);
internal ASTDecl *parse_decl_union(Parser *p);
internal ASTDecl *parse_decl_typedef(Parser *p);
internal ASTDecl *parse_decl_func_rest(Parser *p, TypeSpec *ret_type, String name,
                                        StorageClass sc, u32 line, u32 col);
internal ASTDecl *parse_decl_aggregate(Parser *p, b32 is_struct);

/* ======================================================================
 * Top-level
 * ====================================================================== */

internal ASTDecl **parse_translation_unit(Parser *p);

/* ======================================================================
 * Legacy expression evaluator (kept for existing test_parse_expr)
 * ====================================================================== */

internal s64 expr0(Lexer *lexer);
internal s64 expr1(Lexer *lexer);
internal s64 expr2(Lexer *lexer);
internal s64 expr3(Lexer *lexer);
internal s64 expr4(Lexer *lexer);

#endif /* PARSER_H */
