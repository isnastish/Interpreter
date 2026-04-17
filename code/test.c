

/*
 * Aleksey Yevtushenko
 * February 14, 2022
 */

char *tk_names[]={
    "TK_Eoi",
    "TK_OpenParen",
    "TK_CloseParen",
    "TK_OpenBrace",
    "TK_CloseBrace",
    "TK_OpenBracket",
    "TK_CloseBracket",
    "TK_Hash",
    "TK_Comma",
    "TK_Colon",
    "TK_Semi",
    "TK_Access",
    "TK_Asgn",
    "TK_Less",
    "TK_Grt",
    "TK_Or",
    "TK_And",
    "TK_Xor",
    "TK_Not",
    "TK_Tilde",
    "TK_Plus",
    "TK_Minus",
    "TK_Mul",
    "TK_Div",
    "TK_Mod",
    "TK_BackSlash",
    "TK_At",
    "TK_Dollar",
    "TK_Auto",     "TK_Double", "TK_Int",      "TK_Struct",
    "TK_Break",    "TK_Else",   "TK_Long",     "TK_Switch",
    "TK_Case",     "TK_Enum",   "TK_Register", "TK_Typedef",
    "TK_Char",     "TK_Extern", "TK_Return",   "TK_Union",
    "TK_Const",    "TK_Float",  "TK_Short",    "TK_Unsigned",
    "TK_Continue", "TK_For",    "TK_Signed",   "TK_Void",
    "TK_Default",  "TK_Goto",   "TK_Sizeof",   "TK_Volatile",
    "TK_Do",       "TK_If",     "TK_Static",   "TK_While",
    
    // preprocessor directives.
    "TK_If_dir",
    "TK_ifdef_dir",
    "TK_Ifndef_dir",
    "TK_Elif_dir",
    "TK_Else_dir",
    "TK_Endif_dir",
    "TK_Include_dir",
    "TK_Define_dir",
    "TK_Undef_dir",
    "TK_Line_dir",
    "TK_Error_dir",
    "TK_Pragma_dir",
    
    // special preprocessor directive.
    "TK_defined",
    
    "TK_Deref",
    "TK_Address",
    "TK_OpenAngle",
    "TK_CloseAngle",
    "TK_IntSufU",
    "TK_IntSufL",
    "TK_IntSufUL",
    "TK_IntSufLU",
    "TK_IntSufLLU",
    "TK_IntSufULL",
    "TK_IntSufLL",
    "TK_FloatSufL",
    "TK_FloatSufF",
    "...", "&&", "||", "++", "--", "==", "!=", "<<", ">>",
    "->", "&=", "|=", "^=", "+=", "-=", "*=", "/=",
    "%=", "<=", ">=", "<<=", ">>=",
    "TK_Stringize",
    "TK_Concat",
    "TK_Name",
    "TK_IntNum",
    "TK_FloatNum",
    "TK_Spacing",
    "TK_NewLine",
    "TK_String",
    "TK_Comment",
};

char *lerrors[]={
    "LError_Success",
    "LError_WrongIntSuf",
    "LError_WrongFloatSuf",
    "LError_WrongHexSym",
    "LError_WrongOctSym",
    "LError_WrongBinSym",
    "LError_U64Overflow",
    "LError_F64Overflow",
    "LError_EmptyCharLit",
    "LError_NLInConst",
    "LError_WrongEscSeq",
    "LError_MissedQuote",
    "LError_MissedDQuote",
    "LError_LargeCharLit",
    "LError_ExpDigit",
    "LError_MissedSemicolon",
};

#if 0
void print(Token token){
    if(token.error != LError_Success)
        printf("<%s> ", lerrors[token.error]);
    
    u32 line = (token.linenum);
    u32 column = (token.columnnum);
    
    if(is_token(token, TK_FloatNum))
        printf("(TK_FloatNum, %f) [l:%d, c:%d]\n",
               token.u.F64,
               line,
               column);
    else if(is_token(token, TK_IntNum))
        printf("(TK_IntNum, %llu) [l:%d, c:%d]\n",
               token.u.U64,
               line,
               column);
    else{
        String lexeme = token.lexeme;
        if(token.kind < TK_Auto)
            printf("(%0.*s, '%0.*s') [l:%d, c:%d]\n",
                   lexeme.count,
                   lexeme.data,
                   lexeme.count,
                   lexeme.data,
                   line,
                   column);
        else
            printf("(%s, '%0.*s') [l:%d, c:%d]\n",
                   tk_names[token.kind - TK_Auto],
                   lexeme.count,
                   lexeme.data,
                   line,
                   column);
    }
}
#endif

Lexer partially_init_lexer(String contents){
    Lexer lexer = {0};
    lexer.lastline = 1;
    lexer.lastcolumn = 1;
    lexer.contents = contents;
    refill(&lexer);
    return(lexer);
}

Lexer init_stream(char *stream){
    String contents;
    contents.data = stream;
    contents.count = string_len(stream);
    Lexer lexer = {0};
    lexer.lastline = 1;
    lexer.lastcolumn = 1;
    lexer.contents = contents;
    refill(&lexer);
    return(lexer);
}

/* ======================================================================
 * Parser test helpers
 * ====================================================================== */

internal Parser test_parser_from_string(char *src){
    String contents;
    contents.data = src;
    contents.count = string_len(src);
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    *lexer = init_lexer((String){0}, contents);
    return parser_init(lexer);
}

/* ======================================================================
 * Expression AST tests
 * ====================================================================== */

b32 test_parse_expr_ast(void){
    /* 1 + 2 * 3  =>  BINARY(+, INT(1), BINARY(*, INT(2), INT(3))) */
    {
        Parser p = test_parser_from_string("1 + 2 * 3;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_BINARY);
        Assert(e->binary.op == TK_Plus);
        Assert(e->binary.left->kind == EXPR_INT);
        Assert(e->binary.left->int_lit.val == 1);
        Assert(e->binary.right->kind == EXPR_BINARY);
        Assert(e->binary.right->binary.op == TK_Mul);
        Assert(e->binary.right->binary.left->kind == EXPR_INT);
        Assert(e->binary.right->binary.left->int_lit.val == 2);
        Assert(e->binary.right->binary.right->kind == EXPR_INT);
        Assert(e->binary.right->binary.right->int_lit.val == 3);
    }

    /* -x  =>  UNARY(-, NAME(x)) */
    {
        Parser p = test_parser_from_string("-x;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_UNARY);
        Assert(e->unary.op == TK_Minus);
        Assert(e->unary.operand->kind == EXPR_NAME);
    }

    /* a && b || c  =>  BINARY(||, BINARY(&&, a, b), c) */
    {
        Parser p = test_parser_from_string("a && b || c;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_BINARY);
        Assert(e->binary.op == TK_OrOr);
        Assert(e->binary.left->kind == EXPR_BINARY);
        Assert(e->binary.left->binary.op == TK_AndAnd);
    }

    /* a = b = 5  =>  BINARY(=, a, BINARY(=, b, 5))  (right-associative) */
    {
        Parser p = test_parser_from_string("a = b = 5;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_BINARY);
        Assert(e->binary.op == TK_Asgn);
        Assert(e->binary.left->kind == EXPR_NAME);
        Assert(e->binary.right->kind == EXPR_BINARY);
        Assert(e->binary.right->binary.op == TK_Asgn);
        Assert(e->binary.right->binary.right->kind == EXPR_INT);
        Assert(e->binary.right->binary.right->int_lit.val == 5);
    }

    /* a ? b : c  =>  TERNARY(a, b, c) */
    {
        Parser p = test_parser_from_string("a ? b : c;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_TERNARY);
        Assert(e->ternary.cond->kind == EXPR_NAME);
        Assert(e->ternary.then_expr->kind == EXPR_NAME);
        Assert(e->ternary.else_expr->kind == EXPR_NAME);
    }

    /* f(1, 2)  =>  CALL(NAME(f), [INT(1), INT(2)]) */
    {
        Parser p = test_parser_from_string("f(1, 2);");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_CALL);
        Assert(e->call.func->kind == EXPR_NAME);
        Assert(e->call.num_args == 2);
        Assert(e->call.args[0]->kind == EXPR_INT);
        Assert(e->call.args[0]->int_lit.val == 1);
        Assert(e->call.args[1]->kind == EXPR_INT);
        Assert(e->call.args[1]->int_lit.val == 2);
    }

    /* a[5]  =>  INDEX(NAME(a), INT(5)) */
    {
        Parser p = test_parser_from_string("a[5];");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_INDEX);
        Assert(e->index.array->kind == EXPR_NAME);
        Assert(e->index.index->kind == EXPR_INT);
        Assert(e->index.index->int_lit.val == 5);
    }

    /* p->x  =>  FIELD_PTR(NAME(p), "x") */
    {
        Parser p = test_parser_from_string("p->x;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_FIELD_PTR);
        Assert(e->field.object->kind == EXPR_NAME);
    }

    /* s.y  =>  FIELD(NAME(s), "y") */
    {
        Parser p = test_parser_from_string("s.y;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_FIELD);
        Assert(e->field.object->kind == EXPR_NAME);
    }

    /* x++  =>  POSTFIX(++, NAME(x)) */
    {
        Parser p = test_parser_from_string("x++;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_POSTFIX);
        Assert(e->unary.op == TK_Inc);
        Assert(e->unary.operand->kind == EXPR_NAME);
    }

    /* sizeof(int) => SIZEOF_TYPE */
    {
        Parser p = test_parser_from_string("sizeof(int);");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_SIZEOF_TYPE);
    }

    /* (int)x  =>  CAST(int, NAME(x)) */
    {
        Parser p = test_parser_from_string("(int)x;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_CAST);
        Assert(e->cast.type->kind == TYPESPEC_NAME);
        Assert(e->cast.expr->kind == EXPR_NAME);
    }

    /* a == b != c  =>  BINARY(!=, BINARY(==, a, b), c) */
    {
        Parser p = test_parser_from_string("a == b != c;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_BINARY);
        Assert(e->binary.op == TK_NotEq);
        Assert(e->binary.left->kind == EXPR_BINARY);
        Assert(e->binary.left->binary.op == TK_Eq);
    }

    /* a < b <= c  =>  BINARY(<=, BINARY(<, a, b), c) */
    {
        Parser p = test_parser_from_string("a < b <= c;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_BINARY);
        Assert(e->binary.op == TK_LessEq);
        Assert(e->binary.left->kind == EXPR_BINARY);
        Assert(e->binary.left->binary.op == TK_Less);
    }

    /* a << 2 + 1  =>  BINARY(<<, a, BINARY(+, 2, 1)) */
    {
        Parser p = test_parser_from_string("a << 2 + 1;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_BINARY);
        Assert(e->binary.op == TK_ShiftL);
        Assert(e->binary.right->kind == EXPR_BINARY);
        Assert(e->binary.right->binary.op == TK_Plus);
    }

    /* a += 5  =>  BINARY(+=, NAME(a), INT(5)) */
    {
        Parser p = test_parser_from_string("a += 5;");
        Expr *e = parse_expr(&p);
        Assert(e->kind == EXPR_BINARY);
        Assert(e->binary.op == TK_PlusEq);
    }

    return(1);
}

/* ======================================================================
 * Statement tests
 * ====================================================================== */

b32 test_parse_stmt(void){
    /* if (x) return 1; else return 0; */
    {
        Parser p = test_parser_from_string("if (x) return 1; else return 0;");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_IF);
        Assert(s->if_stmt.cond->kind == EXPR_NAME);
        Assert(s->if_stmt.then_body->kind == STMT_RETURN);
        Assert(s->if_stmt.then_body->ret.expr->kind == EXPR_INT);
        Assert(s->if_stmt.then_body->ret.expr->int_lit.val == 1);
        Assert(s->if_stmt.else_body != 0);
        Assert(s->if_stmt.else_body->kind == STMT_RETURN);
    }

    /* while (i < 10) i++; */
    {
        Parser p = test_parser_from_string("while (i < 10) i++;");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_WHILE);
        Assert(s->while_stmt.cond->kind == EXPR_BINARY);
        Assert(s->while_stmt.cond->binary.op == TK_Less);
        Assert(s->while_stmt.body->kind == STMT_EXPR);
    }

    /* do { x++; } while (x < 5); */
    {
        Parser p = test_parser_from_string("do { x++; } while (x < 5);");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_DO_WHILE);
        Assert(s->do_while_stmt.body->kind == STMT_BLOCK);
        Assert(s->do_while_stmt.cond->kind == EXPR_BINARY);
    }

    /* for (int i = 0; i < 10; i++) { x += i; } */
    {
        Parser p = test_parser_from_string("for (int i = 0; i < 10; i++) { x += i; }");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_FOR);
        Assert(s->for_stmt.init != 0);
        Assert(s->for_stmt.init->kind == STMT_DECL);
        Assert(s->for_stmt.cond != 0);
        Assert(s->for_stmt.post != 0);
        Assert(s->for_stmt.body->kind == STMT_BLOCK);
    }

    /* switch (x) { case 1: break; case 2: return 0; default: break; } */
    {
        Parser p = test_parser_from_string("switch (x) { case 1: break; case 2: return 0; default: break; }");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_SWITCH);
        Assert(s->switch_stmt.num_cases == 3);
        Assert(s->switch_stmt.cases[0].is_default == 0);
        Assert(s->switch_stmt.cases[0].value->kind == EXPR_INT);
        Assert(s->switch_stmt.cases[0].num_stmts == 1);
        Assert(s->switch_stmt.cases[0].stmts[0]->kind == STMT_BREAK);
        Assert(s->switch_stmt.cases[1].is_default == 0);
        Assert(s->switch_stmt.cases[1].num_stmts == 1);
        Assert(s->switch_stmt.cases[1].stmts[0]->kind == STMT_RETURN);
        Assert(s->switch_stmt.cases[2].is_default == 1);
    }

    /* return x + 1; */
    {
        Parser p = test_parser_from_string("return x + 1;");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_RETURN);
        Assert(s->ret.expr->kind == EXPR_BINARY);
    }

    /* return; */
    {
        Parser p = test_parser_from_string("return;");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_RETURN);
        Assert(s->ret.expr == 0);
    }

    /* break; */
    {
        Parser p = test_parser_from_string("break;");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_BREAK);
    }

    /* continue; */
    {
        Parser p = test_parser_from_string("continue;");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_CONTINUE);
    }

    /* goto end; */
    {
        Parser p = test_parser_from_string("goto end;");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_GOTO);
    }

    /* end: return 0; */
    {
        Parser p = test_parser_from_string("end: return 0;");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_LABEL);
        Assert(s->label_stmt.stmt->kind == STMT_RETURN);
    }

    /* { int x = 0; x++; } */
    {
        Parser p = test_parser_from_string("{ int x = 0; x++; }");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_BLOCK);
        Assert(s->block.num_stmts == 2);
        Assert(s->block.stmts[0]->kind == STMT_DECL);
        Assert(s->block.stmts[1]->kind == STMT_EXPR);
    }

    /* Empty statement: ; */
    {
        Parser p = test_parser_from_string(";");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_EXPR);
        Assert(s->expr.expr == 0);
    }

    /* Nested if-else-if */
    {
        Parser p = test_parser_from_string("if (a) x = 1; else if (b) x = 2; else x = 3;");
        Stmt *s = parse_stmt(&p);
        Assert(s->kind == STMT_IF);
        Assert(s->if_stmt.else_body != 0);
        Assert(s->if_stmt.else_body->kind == STMT_IF);
        Assert(s->if_stmt.else_body->if_stmt.else_body != 0);
    }

    return(1);
}

/* ======================================================================
 * Variable declaration tests
 * ====================================================================== */

b32 test_parse_decl_var(void){
    /* int x; */
    {
        Parser p = test_parser_from_string("int x;");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_VAR);
        Assert(d->var.init == 0);
    }

    /* int x = 5; */
    {
        Parser p = test_parser_from_string("int x = 5;");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_VAR);
        Assert(d->var.init != 0);
        Assert(d->var.init->kind == EXPR_INT);
        Assert(d->var.init->int_lit.val == 5);
    }

    /* int *p; */
    {
        Parser p = test_parser_from_string("int *p;");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_VAR);
        Assert(d->var.type->kind == TYPESPEC_PTR);
        Assert(d->var.type->ptr.base->kind == TYPESPEC_NAME);
    }

    /* const int x = 10; */
    {
        Parser p = test_parser_from_string("const int x = 10;");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_VAR);
        Assert(d->var.type->kind == TYPESPEC_CONST);
        Assert(d->var.init != 0);
        Assert(d->var.init->int_lit.val == 10);
    }

    /* unsigned long long x = 42; */
    {
        Parser p = test_parser_from_string("unsigned long long x = 42;");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_VAR);
        Assert(d->var.init->int_lit.val == 42);
    }

    /* static int count = 0; */
    {
        Parser p = test_parser_from_string("static int count = 0;");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_VAR);
        Assert(d->storage == STORAGE_STATIC);
    }

    /* extern int global_var; */
    {
        Parser p = test_parser_from_string("extern int global_var;");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_VAR);
        Assert(d->storage == STORAGE_EXTERN);
        Assert(d->var.init == 0);
    }

    /* int arr[10]; */
    {
        Parser p = test_parser_from_string("int arr[10];");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_VAR);
        Assert(d->var.type->kind == TYPESPEC_ARRAY);
        Assert(d->var.type->array.size->kind == EXPR_INT);
        Assert(d->var.type->array.size->int_lit.val == 10);
    }

    /* double **pp; */
    {
        Parser p = test_parser_from_string("double **pp;");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_VAR);
        Assert(d->var.type->kind == TYPESPEC_PTR);
        Assert(d->var.type->ptr.base->kind == TYPESPEC_PTR);
    }

    return(1);
}

/* ======================================================================
 * Function declaration tests
 * ====================================================================== */

b32 test_parse_decl_func(void){
    /* int add(int a, int b) { return a + b; } */
    {
        Parser p = test_parser_from_string("int add(int a, int b) { return a + b; }");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_FUNC);
        Assert(d->func.num_params == 2);
        Assert(d->func.has_varargs == 0);
        Assert(d->func.body != 0);
        Assert(d->func.body->kind == STMT_BLOCK);
        Assert(d->func.body->block.num_stmts == 1);
        Assert(d->func.body->block.stmts[0]->kind == STMT_RETURN);
    }

    /* void noop(void); */
    {
        Parser p = test_parser_from_string("void noop(void);");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_FUNC);
        Assert(d->func.num_params == 0);
        Assert(d->func.body == 0);
    }

    /* int printf(const char *fmt, ...); */
    {
        Parser p = test_parser_from_string("int printf(const char *fmt, ...);");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_FUNC);
        Assert(d->func.num_params == 1);
        Assert(d->func.has_varargs == 1);
        Assert(d->func.body == 0);
    }

    /* static void helper(int x) { x++; } */
    {
        Parser p = test_parser_from_string("static void helper(int x) { x++; }");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_FUNC);
        Assert(d->storage == STORAGE_STATIC);
        Assert(d->func.num_params == 1);
        Assert(d->func.body != 0);
    }

    /* int main(int argc, char **argv) { return 0; } */
    {
        Parser p = test_parser_from_string("int main(int argc, char **argv) { return 0; }");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_FUNC);
        Assert(d->func.num_params == 2);
        Assert(d->func.params[1].type->kind == TYPESPEC_PTR);
        Assert(d->func.body != 0);
    }

    /* void empty(void) {} */
    {
        Parser p = test_parser_from_string("void empty(void) {}");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_FUNC);
        Assert(d->func.body != 0);
        Assert(d->func.body->kind == STMT_BLOCK);
        Assert(d->func.body->block.num_stmts == 0);
    }

    return(1);
}

/* ======================================================================
 * Struct declaration tests
 * ====================================================================== */

b32 test_parse_decl_struct(void){
    /* struct Point { int x; int y; }; */
    {
        Parser p = test_parser_from_string("struct Point { int x; int y; };");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_STRUCT);
        Assert(d->aggregate.num_fields == 2);
    }

    /* struct { float r; float g; float b; }; (anonymous) */
    {
        Parser p = test_parser_from_string("struct { float r; float g; float b; };");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_STRUCT);
        Assert(d->aggregate.num_fields == 3);
        Assert(d->aggregate.name.count == 0);
    }

    /* struct Node { int val; struct Node *next; }; */
    {
        Parser p = test_parser_from_string("struct Node { int val; struct Node *next; };");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_STRUCT);
        Assert(d->aggregate.num_fields == 2);
    }

    /* struct S; (forward declaration) */
    {
        Parser p = test_parser_from_string("struct S;");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_STRUCT);
        Assert(d->aggregate.num_fields == 0);
    }

    /* struct with pointer member: struct Buf { char *data; int len; }; */
    {
        Parser p = test_parser_from_string("struct Buf { char *data; int len; };");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_STRUCT);
        Assert(d->aggregate.num_fields == 2);
        Assert(d->aggregate.fields[0].type->kind == TYPESPEC_PTR);
    }

    return(1);
}

/* ======================================================================
 * Union declaration tests
 * ====================================================================== */

b32 test_parse_decl_union(void){
    /* union Value { int i; float f; char *s; }; */
    {
        Parser p = test_parser_from_string("union Value { int i; float f; char *s; };");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_UNION);
        Assert(d->aggregate.num_fields == 3);
    }

    /* union { double d; unsigned long long u; }; (anonymous) */
    {
        Parser p = test_parser_from_string("union { double d; unsigned long long u; };");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_UNION);
        Assert(d->aggregate.num_fields == 2);
        Assert(d->aggregate.name.count == 0);
    }

    return(1);
}

/* ======================================================================
 * Enum declaration tests
 * ====================================================================== */

b32 test_parse_decl_enum(void){
    /* enum Color { RED, GREEN, BLUE }; */
    {
        Parser p = test_parser_from_string("enum Color { RED, GREEN, BLUE };");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_ENUM);
        Assert(d->enumeration.num_items == 3);
        Assert(d->enumeration.items[0].value == 0);
        Assert(d->enumeration.items[1].value == 0);
        Assert(d->enumeration.items[2].value == 0);
    }

    /* enum with explicit values: enum Err { OK = 0, FAIL = -1, RETRY = 2 }; */
    {
        Parser p = test_parser_from_string("enum Err { OK = 0, FAIL = 1, RETRY = 2 };");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_ENUM);
        Assert(d->enumeration.num_items == 3);
        Assert(d->enumeration.items[0].value != 0);
        Assert(d->enumeration.items[0].value->kind == EXPR_INT);
        Assert(d->enumeration.items[0].value->int_lit.val == 0);
        Assert(d->enumeration.items[2].value->int_lit.val == 2);
    }

    /* enum with trailing comma: enum { A, B, C, }; */
    {
        Parser p = test_parser_from_string("enum { A, B, C, };");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_ENUM);
        Assert(d->enumeration.num_items == 3);
        Assert(d->enumeration.name.count == 0);
    }

    return(1);
}

/* ======================================================================
 * Typedef tests
 * ====================================================================== */

b32 test_parse_decl_typedef(void){
    /* typedef unsigned int uint; */
    {
        Parser p = test_parser_from_string("typedef unsigned int uint;");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_TYPEDEF);
    }

    /* typedef int *intptr; */
    {
        Parser p = test_parser_from_string("typedef int *intptr;");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_TYPEDEF);
        Assert(d->type_def.type->kind == TYPESPEC_PTR);
    }

    /* typedef const char *cstr; */
    {
        Parser p = test_parser_from_string("typedef const char *cstr;");
        ASTDecl *d = parse_decl(&p);
        Assert(d->kind == DECL_TYPEDEF);
        Assert(d->type_def.type->kind == TYPESPEC_PTR);
    }

    return(1);
}

/* ======================================================================
 * Translation unit test — multiple declarations
 * ====================================================================== */

b32 test_parse_translation_unit(void){
    char *src =
        "int global_x;\n"
        "static int counter = 0;\n"
        "struct Point { int x; int y; };\n"
        "enum Color { RED, GREEN, BLUE };\n"
        "typedef unsigned int uint;\n"
        "int add(int a, int b) { return a + b; }\n"
        "void noop(void);\n";

    Parser p = test_parser_from_string(src);
    ASTDecl **decls = parse_translation_unit(&p);
    Assert(!p.had_error);
    Assert(darr_len(decls) == 7);
    Assert(decls[0]->kind == DECL_VAR);
    Assert(decls[1]->kind == DECL_VAR);
    Assert(decls[1]->storage == STORAGE_STATIC);
    Assert(decls[2]->kind == DECL_STRUCT);
    Assert(decls[3]->kind == DECL_ENUM);
    Assert(decls[4]->kind == DECL_TYPEDEF);
    Assert(decls[5]->kind == DECL_FUNC);
    Assert(decls[5]->func.body != 0);
    Assert(decls[6]->kind == DECL_FUNC);
    Assert(decls[6]->func.body == 0);

    return(1);
}

/* ======================================================================
 * Combined test runner for declarations (called from main.c)
 * ====================================================================== */

b32 test_parse_declarations(void){
    if(!test_parse_expr_ast()) return 0;
    if(!test_parse_stmt()) return 0;
    if(!test_parse_decl_var()) return 0;
    if(!test_parse_decl_func()) return 0;
    if(!test_parse_decl_struct()) return 0;
    if(!test_parse_decl_union()) return 0;
    if(!test_parse_decl_enum()) return 0;
    if(!test_parse_decl_typedef()) return 0;
    if(!test_parse_translation_unit()) return 0;
    return(1);
}

#define dotest(stream, result)\
lexer = init_stream(#stream);\
Assert(expr0(&lexer) == (result));

b32 test_parse_expr(void){
    Lexer lexer;
    
    dotest(0;, 0);
    dotest(1;, 1);
    dotest(+2;, 2);
    dotest(-4;, -4);
    dotest(0 | 1;, 1);
    dotest(0 ^ 2;, 2);
    dotest(0 << 5;, 0);
    dotest(1 + (2 * 3);, 7);
    dotest((9 / 3) + (-5 * 4);, -17);
    dotest(1 + 2 + 3*4 - 20 + -5 * (6 + 4) - -2 + +4;, -49);
    dotest(1 & 3;, 1);
    dotest(1 | 3;, 3); 
    dotest(1 ^ 3;, 2);
    dotest(1 << 4;, 16);
    dotest(16 >> 4;, 1);
    dotest(20 >> 1;, 10);
    dotest(20 | 5 + 6 & 4;, 0);
    dotest((20 | 5) ^ (5 & 15);, 16);
    dotest(1 + 2 * 3 & 5;, 5);
    
    return(1);
}
#undef dotest

Token eattoken(Lexer *lexer){
    Token result = gettoken(lexer);
    return(result);
}

#define Match(match)                                    \
Assert((token = eattoken(&lexer)).kind == match)

b32 test_keywords(void){
    String input;
    input.data =
        "#ifndef(KEYWORDS_H)\r\n"
        "#define KEYWORDS_H\r\n"
        "\r\n\r\n"
        "auto c = 1;\r\n"
        "const volatile unsigned int = 1;\r\n"
        "register char c = 12;\r\n"
        "static short s = 1;\r\n"
        "\r\n"
        "void f(void){\r\n"
        "  while(1);\r\n"
        "  for(int i = 0;;++i){\r\n"
        "    if(i = 10) continue;\r\n"
        "    else if(i == 100) break;\r\n"
        "}\r\n"
        "  long a = 11;\r\n"
        "  switch(a){\r\n"
        "    case(1):break;\r\n"
        "    default: goto end;\r\n"
        "  }\r\n"
        "  char c = malloc(sizeof(char));\r\n"
        "  end:\r\n"
        "  return;\r\n"
        "}\r\n"
        "\r\n\r\n"
        "typedef struct v2{float x1; float x2;}v2;\r\n"
        "typedef enum{token1, token2,};\r\n"
        "typedef union{int a; float b;}\r\n"
        "\r\n\r\n"
        "#endif /* KEYWORDS_H */\r\n";
    input.count = string_len(input.data);
    
    Token token;
    Lexer lexer = partially_init_lexer(input);
    
    // NOTE: test start.
    Match('#');
    Match(TK_Ifndef_dir);
    Match('('); 
    Match(TK_Name);
    Match(')');
    Match('#'); 
    Match(TK_Define_dir);
    Match(TK_Name); 
    Match(TK_Auto);
    Match(TK_Name);
    Match('='); 
    Match(TK_IntNum);
    Match(';');
    Match(TK_Const);
    Match(TK_Volatile);
    Match(TK_Unsigned);
    Match(TK_Int);
    Match('='); 
    Match(TK_IntNum);
    Match(';');
    Match(TK_Register);
    Match(TK_Char);
    Match(TK_Name);
    Match('='); 
    Match(TK_IntNum);
    Match(';');
    Match(TK_Static);
    Match(TK_Short); 
    Match(TK_Name);
    Match('='); 
    Match(TK_IntNum);
    Match(';');
    Match(TK_Void);
    Match(TK_Name);
    Match('('); 
    Match(TK_Void);
    Match(')'); 
    Match('{');
    Match(TK_While);
    Match('('); 
    Match(TK_IntNum);
    Match(')'); 
    Match(';');
    Match(TK_For);
    Match('('); 
    Match(TK_Int); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); 
    Match(';'); 
    Match(';'); 
    Match(TK_Inc); 
    Match(TK_Name); 
    Match(')'); 
    Match('{');
    Match(TK_If); 
    Match('('); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); 
    Match(')'); 
    Match(TK_Continue); 
    Match(';');
    Match(TK_Else); 
    Match(TK_If); 
    Match('('); 
    Match(TK_Name); 
    Match(TK_Eq); 
    Match(TK_IntNum); 
    Match(')'); 
    Match(TK_Break); 
    Match(';'); 
    Match('}');
    Match(TK_Long); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); 
    Match(';');
    Match(TK_Switch); 
    Match('('); 
    Match(TK_Name); 
    Match(')'); 
    Match('{');
    Match(TK_Case); 
    Match('('); 
    Match(TK_IntNum); 
    Match(')'); 
    Match(':'); 
    Match(TK_Break); 
    Match(';');
    Match(TK_Default); 
    Match(':'); 
    Match(TK_Goto); 
    Match(TK_Name); 
    Match(';');
    Match('}');
    Match(TK_Char); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_Name); 
    Match('('); 
    Match(TK_Sizeof); 
    Match('('); 
    Match(TK_Char); 
    Match(')'); 
    Match(')'); 
    Match(';');
    Match(TK_Name); 
    Match(':');
    Match(TK_Return); 
    Match(';');
    Match('}');
    Match(TK_Typedef); 
    Match(TK_Struct); 
    Match(TK_Name); 
    Match('{'); 
    Match(TK_Float); 
    Match(TK_Name); 
    Match(';'); 
    Match(TK_Float); 
    Match(TK_Name); 
    Match(';'); 
    Match('}'); 
    Match(TK_Name); 
    Match(';');
    Match(TK_Typedef); 
    Match(TK_Enum); 
    Match('{'); 
    Match(TK_Name); 
    Match(','); 
    Match(TK_Name); 
    Match(','); 
    Match('}'); 
    Match(';');
    Match(TK_Typedef); 
    Match(TK_Union); 
    Match('{'); 
    Match(TK_Int); 
    Match(TK_Name); 
    Match(';'); 
    Match(TK_Float); 
    Match(TK_Name); 
    Match(';'); 
    Match('}'); 
    Match('#');
    Match(TK_Endif_dir); 
    Match(TK_Eoi);
    
    return(1);
}

b32 test_operators(void){
    String input;
    input.data =
        "#ifndef(OPERATORS_H)\n"
        "#define OPERATORS_H\n"
        "\n\n"
        "void error(char *ft, ...)\n"
        "{\n"
        "  int a = 0, b = 5;\n"
        "  if(a || b) a = (a + b);\n"
        "  else if(a && b) b = a;\n"
        "\n"
        "  if(a == b) a = b;\n"
        "  if(a != b) a = b + 1;\n"
        "\n"
        "  if(a <= b) a = b;\n"
        "  if(a >= b) b = a + 1;\n"
        "\n"
        "  struct v2{float x1; float x2;}v2;\n"
        "  v2 *v = malloc(sizeof(v2));\n"
        "  v->x1 = 0;\n"
        "  a = a << 4;\n"
        "  b = b >> 1;\n"
        "  a <<= 1;\n"
        "  b >>= 2;\n"
        "\n"
        "  ++a;\n"
        "  --b;\n"
        "}\n"
        "\n\n"
        "#endif /* OPERATORS_H */\n";
    input.count = string_len(input.data);
    
    Token token;
    Lexer lexer = partially_init_lexer(input);
    
    // NOTE: test start. 
    Match('#'); 
    Match(TK_Ifndef_dir); 
    Match('('); 
    Match(TK_Name); 
    Match(')');    
    Match('#'); 
    Match(TK_Define_dir); 
    Match(TK_Name);
    Match(TK_Void); 
    Match(TK_Name); 
    Match('('); 
    Match(TK_Char); 
    Match('*'); 
    Match(TK_Name); 
    Match(','); 
    Match(TK_Dots); 
    Match(')');
    Match('{');
    Match(TK_Int); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); 
    Match(','); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); 
    Match(';');
    Match(TK_If); 
    Match('('); 
    Match(TK_Name); 
    Match(TK_OrOr); 
    Match(TK_Name); 
    Match(')'); 
    Match(TK_Name); 
    Match('='); 
    Match('('); 
    Match(TK_Name); 
    Match('+'); 
    Match(TK_Name); 
    Match(')'); 
    Match(';');
    Match(TK_Else); 
    Match(TK_If); 
    Match('('); 
    Match(TK_Name); 
    Match(TK_AndAnd); 
    Match(TK_Name); 
    Match(')'); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_Name); 
    Match(';');
    Match(TK_If); 
    Match('('); 
    Match(TK_Name); 
    Match(TK_Eq); 
    Match(TK_Name); 
    Match(')'); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_Name); 
    Match(';');
    Match(TK_If); 
    Match('('); 
    Match(TK_Name); 
    Match(TK_NotEq); 
    Match(TK_Name); 
    Match(')'); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_Name); 
    Match('+'); 
    Match(TK_IntNum); 
    Match(';');
    Match(TK_If); 
    Match('('); 
    Match(TK_Name); 
    Match(TK_LessEq); 
    Match(TK_Name); 
    Match(')'); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_Name); 
    Match(';');
    Match(TK_If); 
    Match('('); 
    Match(TK_Name); 
    Match(TK_GrtEq); 
    Match(TK_Name); 
    Match(')'); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_Name); 
    Match('+'); 
    Match(TK_IntNum); 
    Match(';');
    Match(TK_Struct); 
    Match(TK_Name); 
    Match('{'); 
    Match(TK_Float); 
    Match(TK_Name); 
    Match(';'); 
    Match(TK_Float); 
    Match(TK_Name); 
    Match(';'); 
    Match('}'); 
    Match(TK_Name); 
    Match(';');
    Match(TK_Name); 
    Match('*'); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_Name); 
    Match('('); 
    Match(TK_Sizeof); 
    Match('('); 
    Match(TK_Name); 
    Match(')'); 
    Match(')'); 
    Match(';');
    Match(TK_Name); 
    Match(TK_Arrow); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); 
    Match(';');
    Match(TK_Name); 
    Match('='); 
    Match(TK_Name); 
    Match(TK_ShiftL); 
    Match(TK_IntNum); 
    Match(';');
    Match(TK_Name); 
    Match('='); 
    Match(TK_Name); 
    Match(TK_ShiftR); 
    Match(TK_IntNum); 
    Match(';');
    Match(TK_Name); 
    Match(TK_ShiftLEq); 
    Match(TK_IntNum); 
    Match(';');
    Match(TK_Name); 
    Match(TK_ShiftREq); 
    Match(TK_IntNum); 
    Match(';');
    Match(TK_Inc); 
    Match(TK_Name); 
    Match(';');
    Match(TK_Dec); 
    Match(TK_Name); 
    Match(';');
    Match('}');
    Match('#');
    Match(TK_Endif_dir);
    Match(TK_Eoi);
    
    return(1);
}

b32 test_integers(void){
    String input;
    input.data =
        "#ifndef(INTS_H)\n"
        "#define INTS_H\n"
        "\n\n"
        "\n"
        "int a = 0;\n"
        "int b = 0b0;\n"
        "int c = 0x0;\n"
        "int d = 0X0;\n"
        "int e = 00;\n"
        "int f = 0B0;\n"
        "u64 i0 = 18446744073709551615;\n" // max value.
        "u64 i1 = 18446744073709551616;\n" // overflow.
        "u64 i2 = 12344;\n"
        "u64 i3 = 0xaa;\n"
        "u64 i4 = 0xbbb;\n"
        "u64 i5 = 0xc3545fddff;\n"
        "u64 i6 = 0xfffffffffffffff8;\n"
        "u64 i7 = 0xffffffffffffffff;\n"
        "u64 i8 = 0xffffffffffffffff7;\n" // overflow
        "u64 i9 = 0xffffffffffffffffa;\n" // overflow
        "u64 i10 = 88787878787878;\n"
        "u64 i11 = 0x347845878778;\n"
        "u64 i12 = 0x9999999;\n"
        "u64 i13 = 0x0;\n"
        "/* some multi-line\n"
        "   comment */\n"
        "u64 i14 = 0b11111111111111111111111111111111;\n"
        "u64 i15 = 0b1111111111111111111111111111111111111111111111111111111111111111;\n"
        "u64 i16 = 0b11111111111111111111111111111111111111111111111111111111111111110;\n"
        "u64 i17 = 0b11111111111111111111111111111111111111111111111111111111111111111;\n" // overflow
        "// some single-line comment\n"
        "u64 i18 = 07734434;\n"
        "u64 i19 = 01777777777777777777777;\n"
        "u64 i20 = 017777777777777777777770;\n"
        "u64 i21 = 017777777777777777777776;\n"
        "u64 i22 = 001;\n"
        "u64 i23 = 00;\n"
        "u64 i24 = 0674454;\n"
        "u64 i25 = 077777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777;\n" // overflows
        "\n"
        "\n"
        //"u64 i26 = 0xr;\n" LError_WrongHexSym;
        "#endif /* INTS_H */\n";
    input.count = string_len(input.data);
    
    Token token;
    Lexer lexer = partially_init_lexer(input);
    
    // NOTE: test start.
    Match('#'); 
    Match(TK_Ifndef_dir); 
    Match('('); 
    Match(TK_Name); 
    Match(')');
    Match('#'); 
    Match(TK_Define_dir); 
    Match(TK_Name);
    Match(TK_Int); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0); Match(';');
    Match(TK_Int); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0b0); Match(';');
    Match(TK_Int); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0x0); Match(';');
    Match(TK_Int); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0X0); Match(';');
    Match(TK_Int); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 00); Match(';');
    Match(TK_Int); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0B0); Match(';');
    Match(TK_Name); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 18446744073709551615); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0); Match(';'); // NOTE: overflow.
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 12344); Match(';');
    Match(TK_Name); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0xaa); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0xbbb); Match(';');
    Match(TK_Name); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0xc3545fddff); Match(';'); 
    Match(TK_Name); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0xfffffffffffffff8); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0xffffffffffffffff); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0); Match(';'); // NOTE: overflow.
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0); Match(';'); // NOTE: overflow.
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 88787878787878); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0x347845878778); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0x9999999); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0b11111111111111111111111111111111); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0b1111111111111111111111111111111111111111111111111111111111111111); Match(';');
    Match(TK_Name); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0); Match(';'); 
    Match(TK_Name); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 07734434); Match(';');
    Match(TK_Name); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 01777777777777777777777); Match(';');
    Match(TK_Name); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0); Match(';');
    Match(TK_Name);
    Match(TK_Name);
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 001); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 00); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0674454); Match(';');
    Match(TK_Name);
    Match(TK_Name); 
    Match('='); 
    Match(TK_IntNum); Assert(token.u.U64 == 0); Match(';');
    Match('#'); 
    Match(TK_Endif_dir);
    Match(TK_Eoi);
    
    return(1);
}

b32 test_floats(void){
    String input;
    input.data =
        "#ifndef(FLOATS_H)\n"
        "#define FLOATS_H\n"
        "\n\n"
        "{\n"
        "\n"
        "  double d0 = 0.0;\n"
        "  double d1 = 0.;\n"
        "  double d2 = 0.1234;\n"
        "  double d3 = 12.3e-1;\n"
        "  double d4 = .01;\n"
        "  double d5 = .0;\n"
        "  double d6 = 1.23;\n"
        "  double d7 = 2.34434;\n"
        "  double d8 = 10.;\n"
        "  double d9 = 8.;\n"
        "  double d10 = 1.3e1;\n"
        "  double d11 = 1.44e+2;\n"
        "  double d12 = 1.12E4;\n"
        "  double d13 = 1.12e-2;\n"
        "  double d14 = 1.23444E+03;\n"
        "  double d15 = .0124;\n"
        "  double d16 = .013E+2;\n"
        "}\n"
        "\n\n"
        "#endif /* FLOATS_H */";
    input.count = string_len(input.data);
    
    Token token;
    Lexer lexer = partially_init_lexer(input);
    
    // NOTE: test start.
    Match('#'); 
    Match(TK_Ifndef_dir); 
    Match('('); 
    Match(TK_Name); 
    Match(')');
    Match('#'); 
    Match(TK_Define_dir); 
    Match(TK_Name); 
    Match('{');
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 0.0); 
    Match(';');
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 0.); 
    Match(';'); 
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 0.1234); 
    Match(';');
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 12.3e-1); 
    Match(';');
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == .01); 
    Match(';');
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == .0); 
    Match(';');
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 1.23); 
    Match(';');
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 2.34434); 
    Match(';');
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 10.); 
    Match(';'); 
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 8.); 
    Match(';'); 
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 1.3e1); 
    Match(';');
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 1.44e+2); 
    Match(';'); 
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 1.12E4); 
    Match(';');
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 1.12e-2); 
    Match(';'); 
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == 1.23444E+03); 
    Match(';'); 
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == .0124); 
    Match(';');
    Match(TK_Double); 
    Match(TK_Name); 
    Match('='); 
    Match(TK_FloatNum); Assert(token.u.F64 == .013E+2); 
    Match(';'); 
    Match('}');
    Match('#'); 
    Match(TK_Endif_dir);
    Match(TK_Eoi);
    
    return(1);
}

b32 test_characters(void){
    String input;
    input.data =
        "#ifndef(CHAR_H)\n"
        "#define CHAR_H\n"
        "\n\n"
        "{\n"
        "  char i = '';\n"        // error: LError_EmptyCharLit.
        "  char i0 = 'atatat';\n" // error: LError_LargeCharLit.
        "  char i1 = '\n';\n"     // error: LError_NLInCharLit.
        "  char i2 = ';\n"        // error: LError_MissedQuote.
        "  char c0 = 'a';\n"
        "  char c1 = 'A';\n"
        "  char c2 = 'f';\n"
        "  char c3 = '5';\n"
        "  char c4 = '0';\n"
        "  char c5 = '\\n';\n"
        "  char c6 = '\\r';\n"
        "  char c7 = '\\t';\n"
        "  char c8 = '\\v';\n"
        "  char c9 = '\\f';\n"
        "  char c10 = '\\b';\n"
        "  char c11 = '\\a';\n"
        "  char c12 = '\\?';\n"
        "  char c13 = '\\'';\n"
        "  char c14 = '\\\"';\n"
        "  char c15 = '\\\\';\n"
        //"  char c16 = '\\xff';\n"
        "  char c17 = '\\0';\n"
        //"  char c18 = '\\x1f';\n"
        //"  char c19 = '\\0244';\n"
        //"  char c20 = '\\034';\n"
        "\n\n"
        "#endif /* CHAR_H */\n";
    input.count = string_len(input.data);
    
    Token token;
    Lexer lexer = partially_init_lexer(input);
    
    // NOTE: test start.
    Match('#');
    Match(TK_Ifndef_dir);
    Match('(');
    Match(TK_Name);
    Match(')'); 
    Match('#');
    Match(TK_Define_dir);
    Match(TK_Name);
    Match('{');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match('#');
    Match(TK_Endif_dir);
    Match(TK_Eoi);
    
    return(1);
}


b32 test_strings(void){
    String input;
    input.data =
        "#ifndef(STRING_H)\n"
        "#define STRING_H\n"
        "\n\n"
        "{\n"
        "char *ss = \"half1\" // comment\n"
        "    // comment\n"
        "    \"half2\"        // comment\n"
        "    /*\n"
        "     *comment\n"
        "     *\n"
        "     */\n"
        "    \"half3\";\n"
        "\n"
        "  char *ss1 = \"half1\" /* the first half. */\n"
        "     \"half2\" /* the second half. */\n"
        "  /* whitesapces. */\n"
        "  /* whitespaces. */\n"
        "\n"
        "\n"
        "\n"
        "    \"last half\";\n"
        "\n"
        "\n"
        "  char *ss0 = \"part1\"\n"
        "     \"part2\"\n"
        "     \"part3\"\n"
        "     \"part4\"\n"
        "\n"
        "\n"
        "    \"last part\";\n"
        "  char *s = \"part1\"   \"part2\";\n" /* crazy string. */
        "  char *s = \"s2\"\n"
        "      ;\n"          /* ok, semicolon to the previous declaration. */
        "  char *s0 = \"s0\"\n"
        "\n"
        "\n"
        "\n"
        "     ;\n"
        "  char *ppp = \"wrong \\# escape sequence.\";\n" /* error: LError_WrongEscSeq. */
        "  char *pp = \"New line in string \n constant.\";\n"  /* error: LError_NLInConst. */
        "  char *p = \"this is a string\";\n"
        "  char *p1 = \"this is \\\"nested\\\" string\";\n"
        //"  char *p7 = \"string contains \n new line\";\n"                                    // syntax_error: Windows, Unix.
        //"  char *p8 = \"string contains \r carriage return, which is not allowed.\";\n"        // syntax_error: Macintosh.
        "  char *p9 = \"string with \\n escape sequence\";\n"
        "  char *p10 = \"string with \\r escape sequence\";\n"
        "  char *p11 = \"string with unrecognized \\a escape sequence\";\n"
        "  char *p12 = \"string with \\0 inside string\";\n"
        "  char *p13 = \"\\n\";\n"
        "  char *p14 = \"\\r\";\n"
        "  char *p15 = \"\\t\";\n"
        "  char *p16 = \"\\v\";\n"
        "  char *p17 = \"\\f\";\n"
        "  char *p18 = \"\\b\";\n"
        "  char *p19 = \"\\a\";\n"
        "  char *p20 = \"\\\\\";\n"
        "  char *p21 = \"\\'\";\n"
        "  char *p22 = \"\\\"\";\n"
        "  char *p23 = \"\\?\";\n"
        "  char *p24 = \"\\0\";\n"
        //"  char *p25 = \"\\B\";\n"
        //"  char *p26 = \"\\7\";\n"
        "}\n"
        "\n\n"
        "#endif // STRING_H\n";
    input.count = string_len(input.data);
    Token token;
    Lexer lexer = partially_init_lexer(input);
    
    // NOTE: test start.
    Match('#'); Match(TK_Ifndef_dir); Match('('); Match(TK_Name); Match(')');
    Match('#'); Match(TK_Define_dir); Match(TK_Name); 
    Match('{');
    Match(TK_Char); Match('*'); Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';'); 
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';'); 
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';'); 
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';'); 
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match(TK_Char);
    Match('*');
    Match(TK_Name);
    Match('=');
    Match(TK_String);
    Match(';');
    Match('}');
    Match('#');
    Match(TK_Endif_dir);
    Match(TK_Eoi);
    
    return(1);
}

b32 test_comments(void){
    String input;
    input.data =
        "#if !defined(COMMENTS_H)\n"
        "#define COMMENTS_H\n"
        "\n\n"
        "{\n"
        "  // single-line comment.\n"
        "  int a; // uninitialized variable.\n"
        "  char arr[]={1, 3, /*end the last*/ 5};\n"
        "  /**/ // empty comment.\n"
        "  /*this is\n"
        "    multi-line comment*/\n"
        "  /*\n"
        "\n"
        "\n"
        "\n"
        "  */\n"
        " // that was empty multi line comment.\n"
        "  if(a == 0) a = 1; // assign value to a.\n"
        "}\n"
        "\n\n"
        "#endif // COMMENTS_H\n";
    input.count = string_len(input.data);
    
    Token token;
    Lexer lexer = partially_init_lexer(input);
    
    // NOTE: test start.
    Match('#');
    Match(TK_If_dir);
    Match('!');
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(')');
    Match('#');
    Match(TK_Define_dir);
    Match(TK_Name);
    Match('{'); 
    Match(TK_Int);
    Match(TK_Name);
    Match(';');
    Match(TK_Char);
    Match(TK_Name);
    Match('[');
    Match(']');
    Match('=');
    Match('{');
    Match(TK_IntNum);
    Match(',');
    Match(TK_IntNum);
    Match(',');
    Match(TK_IntNum);
    Match('}');
    Match(';'); 
    Match(TK_If);
    Match('(');
    Match(TK_Name);
    Match(TK_Eq);
    Match(TK_IntNum);
    Match(')');
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(';');
    Match('}');
    Match('#');
    Match(TK_Endif_dir);
    Match(TK_Eoi);
    
    return(1);
}

b32 test_suffixes(void){
    String input;
    input.data =
        "#ifndef SUFFIXES_H\n"
        "#define SUFFIXES_H\n"
        "\n"
        "\n"
        "unsigned u0 = 1u;\n"
        "long l0 = 2l;\n"
        "unsigned long ul = 3ul;\n"
        "long unsigned lu = 4lu;\n"
        "long long unsigned llu = 5llu;\n"
        "unsigned long long ull = 6ull;\n"
        "long long ll = 7ll;\n"
        "\n\n"
        "unsigned U0 = 1U;\n"
        "long L0 = 2L;\n"
        "unsigned long UL = 3UL;\n"
        "long unsigned LU = 4LU;\n"
        "long long unsigned LLU = 5LLU;\n"
        "unsigned long long ULL = 6ULL;\n"
        "long long LL = 7LL;\n"
        "\n"
        "long double ld = 5.5l;\n"
        "float f0 = 45e-10f;\n"
        "long double LD = 5.5L;\n"
        "float F = 45e-10F;\n"
        "\n"
        "#endif // SUFFIXES_H\n";
    input.count = string_len(input.data);
    
    Token token;
    Lexer lexer = partially_init_lexer(input);
    
    // NOTE: test start.
    Match('#');
    Match(TK_Ifndef_dir);
    Match(TK_Name);
    Match('#');
    Match(TK_Define_dir);
    Match(TK_Name);    
    Match(TK_Unsigned);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufU);
    Match(';');
    Match(TK_Long);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufL);
    Match(';');
    Match(TK_Unsigned);
    Match(TK_Long);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufUL);
    Match(';');
    Match(TK_Long);
    Match(TK_Unsigned);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufLU);
    Match(';');
    Match(TK_Long);
    Match(TK_Long);
    Match(TK_Unsigned);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufLLU);
    Match(';');
    Match(TK_Unsigned);
    Match(TK_Long);
    Match(TK_Long);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufULL);
    Match(';');
    Match(TK_Long);
    Match(TK_Long);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufLL);
    Match(';');
    Match(TK_Unsigned);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufU);
    Match(';');
    Match(TK_Long);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufL);
    Match(';');
    Match(TK_Unsigned);
    Match(TK_Long);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufUL);
    Match(';');
    Match(TK_Long);
    Match(TK_Unsigned);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufLU);
    Match(';');
    Match(TK_Long);
    Match(TK_Long);
    Match(TK_Unsigned);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufLLU);
    Match(';');
    Match(TK_Unsigned);
    Match(TK_Long);
    Match(TK_Long);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufULL);
    Match(';');
    Match(TK_Long);
    Match(TK_Long);
    Match(TK_Name);
    Match('=');
    Match(TK_IntNum);
    Match(TK_IntSufLL);
    Match(';');
    Match(TK_Long);
    Match(TK_Double);
    Match(TK_Name);
    Match('=');
    Match(TK_FloatNum);
    Match(TK_FloatSufL);
    Match(';');
    Match(TK_Float);
    Match(TK_Name);
    Match('=');
    Match(TK_FloatNum);
    Match(TK_FloatSufF);
    Match(';'); 
    Match(TK_Long);
    Match(TK_Double);
    Match(TK_Name);
    Match('=');
    Match(TK_FloatNum);
    Match(TK_FloatSufL);
    Match(';');
    Match(TK_Float);
    Match(TK_Name);
    Match('=');
    Match(TK_FloatNum);
    Match(TK_FloatSufF);
    Match(';');
    Match('#');
    Match(TK_Endif_dir);
    Match(TK_Eoi);
    
    return(1);
}

b32 test_preproc(void){
    String input;
    input.data = 
        "#if !defined(PREPROC_H)\n"
        "#define PREPROC_H\n"
        "\n"
        "\n"
        "#define cat(x, y) x ## y\n"
        "#define xcat(x, y) cat(x, y)\n"
        "#define min(a, b) ((a) < (b) ? (a) : (b))\n"
        "#define max(a, b) ((a) > (b) ? (a) : (b))\n"
        "#include <stdio.h>\n"
        "#include \"preproc.h\"\n"
        "\n"
        "#if SYSTEM_1\n"
        "#include \"system_1.h\"\n"
        "#elif SYSTEM_2\n"
        "#include \"system_2.h\"\n"
        "#else\n"
        "#include \"system_3.h\"\n"
        "#endif\n"
        "\n"
        "#define const\n" // hiding 'const' keyword.
        "#define BUFFER_SIZE 1024\n"
        "#define NUMBERS 1, \\ \n"
        "                2, \\ \n"
        "                3, \\ \n"
        "                4\n"
        "int x[]={NUMBERS};\n" // int x[]={1, 2, 3, 4};
        "#define TABLESIZE BUFSIZE\n"
        "#define BUFSIZE 1024\n"
        "int y = TABLESIZE;\n" // TABLESIZE->BUFSIZE, BUFSIZE->1024
        "#undef BUFSIZE\n"
        
        "#define BUFSIZE 1020\n"
        "int z = BUFSIZE;\n" /* 1020 */
        "#undef BUFSIZE\n"
        "\n"
        "#define c_entry_point(x, y) int main(int x, char **y)\n"
        "c_entry_point(args_count, args_value);\n" // int main(int args_count, char **args_value);
        "#define foo()\n"                // optimized inline version of foo.
        "foo();\n"                       // will use the macro.
        "void (*funcptr)(void) = foo;\n" // will get an address of the real function.
        "\n"
        "\n"
        
        // new macroses
        "#define SKIP_WHITESPACES(lexer)         \\ \n"
        "    while(is_whitespace(lexer->at[0]))  \\ \n"
        "    advance_chars(lexer, 1)\n"
        "#define stringize(x) #x\n"
        "\n"
        "\n"
        "#define xstr(s) str(s)\n"
        "#define str(s) #s\n"
        "#define four 4\n"
        "#define xcat(x,y) cat(x,y)\n"
        "#define cat(x,y) x##y\n"
        "#define five 5\n"
        
        "#define AFTERX(x) X_ ## x\n"
        "#define XAFTERX(x) AFTERX(x)\n"
        "#define TABLE_SIZE 1024\n"
        "#define BUFSIZE TABLE_SIZE\n"
        
        "\n"
        "\n"
        "#define FAILED_COLOR 0b1100\n"
        "#define PASSED_COLOR 0b0010\n"
        "#define TEST(test)                                                 \\\n"
        "    printf(\"%s: \", #test);                                       \\\n"
        "    if(test()){                                                    \\\n"
        "        SetConsoleTextAttribute(console, PASSED_COLOR);            \\\n"
        "        printf(\"passed!\n\");                                     \\\n"
        "    }                                                              \\\n"
        "    else{                                                          \\\n"
        "        SetConsoleTextAttribute(console, FAILED_COLOR);            \\\n"
        "        printf(\"failed!\n\");                                     \\\n"
        "    }                                                              \\\n"
        "    SetConsoleTextAttribute(console, console_info.wAttributes);\n"
        "\n"
        "\n"
        "\n"
        "#endif /* PREPROC_H*/ \n";
    input.count = string_len(input.data);
    
    Token token;
    Lexer lexer = partially_init_lexer(input);
    
    // NOTE: test start.
    Match(TK_Hash);
    Match(TK_If_dir);
    Match('!');
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(')');
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(',');
    Match(TK_Name);
    Match(')');
    Match(TK_Name);
    Match(TK_Concat);
    Match(TK_Name);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(',');
    Match(TK_Name);
    Match(')');
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(',');
    Match(TK_Name);
    Match(')');
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(',');
    Match(TK_Name);
    Match(')');
    Match('(');
    Match('(');
    Match(TK_Name);
    Match(')');
    Match('<');
    Match('(');
    Match(TK_Name);
    Match(')');
    Match('?');
    Match('(');
    Match(TK_Name);
    Match(')');
    Match(':');
    Match('(');
    Match(TK_Name);
    Match(')');
    Match(')');
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(',');
    Match(TK_Name);
    Match(')');
    Match('(');
    Match('(');
    Match(TK_Name);
    Match(')');
    Match('>');
    Match('(');
    Match(TK_Name);
    Match(')');
    Match('?');
    Match('(');
    Match(TK_Name);
    Match(')');
    Match(':');
    Match('(');
    Match(TK_Name);
    Match(')');
    Match(')');
    Match(TK_Hash);
    Match(TK_Include_dir);
    Match(TK_Less);
    Match(TK_Name);
    Match('.');
    Match(TK_Name);
    Match(TK_Grt);
    Match(TK_Hash);
    Match(TK_Include_dir);
    Match(TK_String);
    Match(TK_Hash);
    Match(TK_If_dir);
    Match(TK_Name);
    Match(TK_Hash);
    Match(TK_Include_dir);
    Match(TK_String);
    Match(TK_Hash);
    Match(TK_Elif_dir);
    Match(TK_Name);
    Match(TK_Hash);
    Match(TK_Include_dir);
    Match(TK_String);
    Match(TK_Hash);
    Match(TK_Else_dir);
    Match(TK_Hash);
    Match(TK_Include_dir);
    Match(TK_String);
    Match(TK_Hash);
    Match(TK_Endif_dir);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Const);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_IntNum);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_IntNum);
    Match(',');
    Match(TK_PPslash);
    Match(TK_IntNum);
    Match(',');
    Match(TK_PPslash);
    Match(TK_IntNum);
    Match(',');
    Match(TK_PPslash);
    Match(TK_IntNum);
    Match(TK_Int);
    Match(TK_Name);
    Match('[');
    Match(']');
    Match('=');
    Match('{');
    Match(TK_Name);
    Match('}');
    Match(';');
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_Name);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_IntNum);
    Match(TK_Int);
    Match(TK_Name);
    Match('=');
    Match(TK_Name);
    Match(';');
    Match(TK_Hash);
    Match(TK_Undef_dir);
    Match(TK_Name);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_IntNum);
    Match(TK_Int);
    Match(TK_Name);
    Match('=');
    Match(TK_Name);
    Match(';');
    Match(TK_Hash);
    Match(TK_Undef_dir);
    Match(TK_Name);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(',');
    Match(TK_Name);
    Match(')');
    Match(TK_Int);
    Match(TK_Name);
    Match('(');
    Match(TK_Int);
    Match(TK_Name);
    Match(',');
    Match(TK_Char);
    Match('*');
    Match('*');
    Match(TK_Name);
    Match(')');
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(',');
    Match(TK_Name);
    Match(')');
    Match(';');
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match('(');
    Match(')');
    Match(TK_Name);
    Match('(');
    Match(')');
    Match(';');
    Match(TK_Void);
    Match('(');
    Match('*');
    Match(TK_Name);
    Match(')');
    Match('(');
    Match(TK_Void);
    Match(')');
    Match('=');
    Match(TK_Name);
    Match(';');
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(')');
    Match(TK_PPslash);
    Match(TK_While);
    Match('(');
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(TK_Arrow);
    Match(TK_Name);
    Match('[');
    Match(TK_IntNum);
    Match(']');
    Match(')');
    Match(')');
    Match(TK_PPslash);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(',');
    Match(TK_IntNum);
    Match(')');
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(')');
    Match(TK_Stringize);
    Match(TK_Name);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(')');
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(')');
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_OpenParen);
    Match(TK_Name);
    Match(TK_CloseParen);
    Match(TK_Stringize);
    Match(TK_Name);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_IntNum);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_OpenParen);
    Match(TK_Name);
    Match(TK_Comma);
    Match(TK_Name);
    Match(TK_CloseParen);
    Match(TK_Name);
    Match(TK_OpenParen);
    Match(TK_Name);
    Match(TK_Comma);
    Match(TK_Name);
    Match(TK_CloseParen);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_OpenParen);
    Match(TK_Name);
    Match(TK_Comma);
    Match(TK_Name);
    Match(TK_CloseParen);
    Match(TK_Name);
    Match(TK_Concat);
    Match(TK_Name);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_IntNum);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_OpenParen);
    Match(TK_Name);
    Match(TK_CloseParen);
    Match(TK_Name);
    Match(TK_Concat);
    Match(TK_Name);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_OpenParen);
    Match(TK_Name);
    Match(TK_CloseParen);
    Match(TK_Name);
    Match(TK_OpenParen);
    Match(TK_Name);
    Match(TK_CloseParen);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_IntNum);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_Name);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_IntNum);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match(TK_IntNum);
    Match(TK_Hash);
    Match(TK_Define_dir);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(')');
    Match(TK_PPslash);
    Match(TK_Name);
    Match('(');
    Match(TK_String);
    Match(',');
    Match(TK_Stringize);
    Match(TK_Name);
    Match(TK_CloseParen);
    Match(TK_Semi);
    Match(TK_PPslash);
    Match(TK_If);
    Match(TK_OpenParen);
    Match(TK_Name);
    Match('(');
    Match(')');
    Match(')');
    Match('{');
    Match(TK_PPslash);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(',');
    Match(TK_Name);
    Match(')');
    Match(';');
    Match(TK_PPslash);
    Match(TK_Name);
    Match('(');
    Match(TK_String);
    Match(')');
    Match(';');
    Match(TK_PPslash);
    Match('}');
    Match(TK_PPslash);
    Match(TK_Else);
    Match('{');
    Match(TK_PPslash);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(',');
    Match(TK_Name);
    Match(')');
    Match(';');
    Match(TK_PPslash);
    Match(TK_Name);
    Match('(');
    Match(TK_String);
    Match(')');
    Match(';');
    Match(TK_PPslash);
    Match('}');
    Match(TK_PPslash);
    Match(TK_Name);
    Match('(');
    Match(TK_Name);
    Match(',');
    Match(TK_Name);
    Match(TK_Access);
    Match(TK_Name);
    Match(')');
    Match(';');
    Match(TK_Hash);
    Match(TK_Endif_dir);
    Match(TK_Eoi);
    
    return(1);
}
#undef Match

#define ANSI_RED    "\033[31m"
#define ANSI_GREEN  "\033[32m"
#define ANSI_RESET  "\033[0m"
#define TEST(test)                                                 \
    printf("%s: ", #test);                                         \
    if(test()){                                                    \
        printf(ANSI_GREEN "passed!" ANSI_RESET "\n");              \
    }                                                              \
    else{                                                          \
        printf(ANSI_RED "failed!" ANSI_RESET "\n");                \
    }

