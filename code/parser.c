
/*
 * parser.c — Full C99 recursive-descent parser.
 *
 * Aleksey Yevtushenko
 * Originally started February 16, 2022 (expression evaluator).
 * Extended with complete C99 AST-building parser.
 */


/* ======================================================================
 * Parser infrastructure
 * ====================================================================== */

internal Parser parser_init(Lexer *lexer){
    Parser p = {0};
    p.lexer = lexer;
    p.had_error = 0;
    p.panic_mode = 0;
    nexttoken(lexer);
    return p;
}

internal void parse_error(Parser *p, const char *msg){
    if(p->panic_mode) return;
    p->panic_mode = 1;
    p->had_error = 1;
    fprintf(stderr, "[line %u, col %u] Parse error: %s\n",
            p->lexer->token.linenumber,
            p->lexer->token.columnnumber, msg);
}

internal void parse_error_at(Parser *p, u32 line, u32 col, const char *msg){
    if(p->panic_mode) return;
    p->panic_mode = 1;
    p->had_error = 1;
    fprintf(stderr, "[line %u, col %u] Parse error: %s\n", line, col, msg);
}

internal void parser_advance(Parser *p){
    nexttoken(p->lexer);
}

internal b32 parser_check(Parser *p, s32 kind){
    return is_token(p->lexer->token, kind);
}

internal b32 parser_match(Parser *p, s32 kind){
    if(parser_check(p, kind)){
        parser_advance(p);
        return 1;
    }
    return 0;
}

internal void parser_expect(Parser *p, s32 kind, const char *msg){
    if(parser_check(p, kind)){
        parser_advance(p);
        return;
    }
    parse_error(p, msg);
}

internal void parser_synchronize(Parser *p){
    p->panic_mode = 0;
    while(!parser_check(p, TK_Eoi)){
        if(parser_check(p, TK_Semi)){
            parser_advance(p);
            return;
        }
        if(parser_check(p, TK_CloseBrace)) return;

        switch(p->lexer->token.kind){
            case TK_If: case TK_While: case TK_For: case TK_Do:
            case TK_Switch: case TK_Return: case TK_Break: case TK_Continue:
            case TK_Goto: case TK_Struct: case TK_Union: case TK_Enum:
            case TK_Typedef: case TK_Extern: case TK_Static:
                return;
            default: break;
        }
        parser_advance(p);
    }
}

/* ======================================================================
 * Type recognition and parsing
 * ====================================================================== */

internal b32 is_type_token(Parser *p){
    switch(p->lexer->token.kind){
        case TK_Void: case TK_Char: case TK_Short: case TK_Int: case TK_Long:
        case TK_Float: case TK_Double: case TK_Signed: case TK_Unsigned:
        case TK_Const: case TK_Volatile:
        case TK_Struct: case TK_Union: case TK_Enum:
        case TK_Static: case TK_Extern: case TK_Auto: case TK_Register:
        case TK_Typedef:
            return 1;
        default:
            return 0;
    }
}

internal TypeSpec *make_named_type(const char *name_str){
    String s = {0};
    s.data = (char *)name_str;
    s.count = string_len(name_str);
    return typespec_name(s);
}

/*
 * Parse a base type, handling multiple specifiers like "unsigned long long int".
 * Returns a TypeSpec (always TYPESPEC_NAME for built-in types).
 */
internal TypeSpec *parse_base_type(Parser *p){
    b32 has_signed = 0, has_unsigned = 0;
    b32 has_short = 0;
    s32 long_count = 0;
    b32 has_int = 0, has_char = 0, has_void = 0;
    b32 has_float = 0, has_double = 0;
    b32 has_const = 0, has_volatile = 0;
    b32 found_specifier = 0;

    for(;;){
        if(parser_check(p, TK_Const)){
            has_const = 1; parser_advance(p);
        }
        else if(parser_check(p, TK_Volatile)){
            has_volatile = 1; parser_advance(p);
        }
        else if(parser_check(p, TK_Signed)){
            has_signed = 1; parser_advance(p); found_specifier = 1;
        }
        else if(parser_check(p, TK_Unsigned)){
            has_unsigned = 1; parser_advance(p); found_specifier = 1;
        }
        else if(parser_check(p, TK_Short)){
            has_short = 1; parser_advance(p); found_specifier = 1;
        }
        else if(parser_check(p, TK_Long)){
            long_count++; parser_advance(p); found_specifier = 1;
        }
        else if(parser_check(p, TK_Int)){
            has_int = 1; parser_advance(p); found_specifier = 1;
        }
        else if(parser_check(p, TK_Char)){
            has_char = 1; parser_advance(p); found_specifier = 1;
        }
        else if(parser_check(p, TK_Void)){
            has_void = 1; parser_advance(p); found_specifier = 1;
        }
        else if(parser_check(p, TK_Float)){
            has_float = 1; parser_advance(p); found_specifier = 1;
        }
        else if(parser_check(p, TK_Double)){
            has_double = 1; parser_advance(p); found_specifier = 1;
        }
        else if(parser_check(p, TK_Struct)){
            parser_advance(p);
            String name = {0};
            if(parser_check(p, TK_Name)){
                name = p->lexer->token.lexeme;
                parser_advance(p);
            }
            AggregateField *fields = 0;
            s32 num_fields = 0;
            if(parser_check(p, TK_OpenBrace)){
                parser_advance(p);
                while(!parser_check(p, TK_CloseBrace) && !parser_check(p, TK_Eoi)){
                    TypeSpec *ft = parse_full_type(p);
                    String fn = {0};
                    if(parser_check(p, TK_Name)){
                        fn = p->lexer->token.lexeme;
                        parser_advance(p);
                    }
                    AggregateField af = {0};
                    af.type = ft;
                    af.name = fn;
                    DynamicArray__fit(fields, 1);
                    fields[darr_len(fields)] = af;
                    DynamicArray__header(fields)->len += 1;
                    num_fields++;
                    parser_expect(p, TK_Semi, "expected ';' after struct field");
                }
                parser_expect(p, TK_CloseBrace, "expected '}' after struct body");
            }
            TypeSpec *st = AST_ALLOC(TypeSpec);
            st->kind = TYPESPEC_STRUCT;
            st->aggregate.name = name;
            st->aggregate.fields = fields;
            st->aggregate.num_fields = num_fields;
            TypeSpec *result = st;
            if(has_const) result = typespec_const(result);
            if(has_volatile) result = typespec_volatile(result);
            return result;
        }
        else if(parser_check(p, TK_Union)){
            parser_advance(p);
            String name = {0};
            if(parser_check(p, TK_Name)){
                name = p->lexer->token.lexeme;
                parser_advance(p);
            }
            AggregateField *fields = 0;
            s32 num_fields = 0;
            if(parser_check(p, TK_OpenBrace)){
                parser_advance(p);
                while(!parser_check(p, TK_CloseBrace) && !parser_check(p, TK_Eoi)){
                    TypeSpec *ft = parse_full_type(p);
                    String fn = {0};
                    if(parser_check(p, TK_Name)){
                        fn = p->lexer->token.lexeme;
                        parser_advance(p);
                    }
                    AggregateField af = {0};
                    af.type = ft;
                    af.name = fn;
                    DynamicArray__fit(fields, 1);
                    fields[darr_len(fields)] = af;
                    DynamicArray__header(fields)->len += 1;
                    num_fields++;
                    parser_expect(p, TK_Semi, "expected ';' after union field");
                }
                parser_expect(p, TK_CloseBrace, "expected '}' after union body");
            }
            TypeSpec *ut = AST_ALLOC(TypeSpec);
            ut->kind = TYPESPEC_UNION;
            ut->aggregate.name = name;
            ut->aggregate.fields = fields;
            ut->aggregate.num_fields = num_fields;
            TypeSpec *result = ut;
            if(has_const) result = typespec_const(result);
            if(has_volatile) result = typespec_volatile(result);
            return result;
        }
        else if(parser_check(p, TK_Enum)){
            parser_advance(p);
            String name = {0};
            if(parser_check(p, TK_Name)){
                name = p->lexer->token.lexeme;
                parser_advance(p);
            }
            EnumItem *items = 0;
            s32 num_items = 0;
            if(parser_check(p, TK_OpenBrace)){
                parser_advance(p);
                while(!parser_check(p, TK_CloseBrace) && !parser_check(p, TK_Eoi)){
                    String item_name = {0};
                    if(parser_check(p, TK_Name)){
                        item_name = p->lexer->token.lexeme;
                        parser_advance(p);
                    }
                    Expr *val = 0;
                    if(parser_match(p, TK_Asgn)){
                        val = parse_expr_ternary(p);
                    }
                    EnumItem ei = {0};
                    ei.name = item_name;
                    ei.value = val;
                    DynamicArray__fit(items, 1);
                    items[darr_len(items)] = ei;
                    DynamicArray__header(items)->len += 1;
                    num_items++;
                    if(!parser_match(p, TK_Comma)) break;
                }
                parser_expect(p, TK_CloseBrace, "expected '}' after enum body");
            }
            TypeSpec *et = AST_ALLOC(TypeSpec);
            et->kind = TYPESPEC_ENUM;
            et->enumeration.name = name;
            et->enumeration.items = items;
            et->enumeration.num_items = num_items;
            TypeSpec *result = et;
            if(has_const) result = typespec_const(result);
            if(has_volatile) result = typespec_volatile(result);
            return result;
        }
        else{
            break;
        }
    }

    TypeSpec *result = 0;
    if(!found_specifier){
        if(parser_check(p, TK_Name)){
            result = typespec_name(p->lexer->token.lexeme);
            parser_advance(p);
        } else {
            parse_error(p, "expected type specifier");
            return make_named_type("int");
        }
    }
    else if(has_void) result = make_named_type("void");
    else if(has_char){
        if(has_unsigned) result = make_named_type("unsigned char");
        else if(has_signed) result = make_named_type("signed char");
        else result = make_named_type("char");
    }
    else if(has_short){
        if(has_unsigned) result = make_named_type("unsigned short");
        else result = make_named_type("short");
    }
    else if(has_float) result = make_named_type("float");
    else if(has_double){
        if(long_count >= 1) result = make_named_type("long double");
        else result = make_named_type("double");
    }
    else if(long_count >= 2){
        if(has_unsigned) result = make_named_type("unsigned long long");
        else result = make_named_type("long long");
    }
    else if(long_count == 1){
        if(has_unsigned) result = make_named_type("unsigned long");
        else result = make_named_type("long");
    }
    else{
        if(has_unsigned) result = make_named_type("unsigned int");
        else result = make_named_type("int");
    }

    if(has_const) result = typespec_const(result);
    if(has_volatile) result = typespec_volatile(result);

    return result;
}

/*
 * Parse a complete type including pointer decorations.
 * type_specifier pointer*
 * pointer := '*' ('const' | 'volatile')*
 */
internal TypeSpec *parse_full_type(Parser *p){
    TypeSpec *base = parse_base_type(p);
    while(parser_check(p, TK_Mul)){
        parser_advance(p);
        base = typespec_ptr(base);
        while(parser_check(p, TK_Const) || parser_check(p, TK_Volatile)){
            if(parser_match(p, TK_Const))    base = typespec_const(base);
            if(parser_match(p, TK_Volatile)) base = typespec_volatile(base);
        }
    }
    return base;
}

/* ======================================================================
 * Expression parsing — full C99 precedence (15 levels)
 * ====================================================================== */

internal b32 is_assign_op(Parser *p){
    switch(p->lexer->token.kind){
        case TK_Asgn: case TK_PlusEq: case TK_MinusEq: case TK_MulEq:
        case TK_DivEq: case TK_ModEq: case TK_AndEq: case TK_OrEq:
        case TK_XorEq: case TK_ShiftLEq: case TK_ShiftREq:
            return 1;
        default: return 0;
    }
}

/* expr := assign (',' assign)* */
internal Expr *parse_expr(Parser *p){
    Expr *left = parse_expr_assign(p);
    while(parser_check(p, TK_Comma)){
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *right = parse_expr_assign(p);
        left = expr_binary(TK_Comma, left, right, line, col);
    }
    return left;
}

/* assign := ternary (assign_op assign)? */
internal Expr *parse_expr_assign(Parser *p){
    Expr *left = parse_expr_ternary(p);
    if(is_assign_op(p)){
        TokenKind op = p->lexer->token.kind;
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *right = parse_expr_assign(p);
        left = expr_binary(op, left, right, line, col);
    }
    return left;
}

/* ternary := logical_or ('?' expr ':' ternary)? */
internal Expr *parse_expr_ternary(Parser *p){
    Expr *cond = parse_expr_logical_or(p);
    if(parser_check(p, '?')){
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *then_expr = parse_expr(p);
        parser_expect(p, TK_Colon, "expected ':' in ternary expression");
        Expr *else_expr = parse_expr_ternary(p);
        cond = expr_ternary(cond, then_expr, else_expr, line, col);
    }
    return cond;
}

/* logical_or := logical_and ('||' logical_and)* */
internal Expr *parse_expr_logical_or(Parser *p){
    Expr *left = parse_expr_logical_and(p);
    while(parser_check(p, TK_OrOr)){
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *right = parse_expr_logical_and(p);
        left = expr_binary(TK_OrOr, left, right, line, col);
    }
    return left;
}

/* logical_and := bitwise_or ('&&' bitwise_or)* */
internal Expr *parse_expr_logical_and(Parser *p){
    Expr *left = parse_expr_bitwise_or(p);
    while(parser_check(p, TK_AndAnd)){
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *right = parse_expr_bitwise_or(p);
        left = expr_binary(TK_AndAnd, left, right, line, col);
    }
    return left;
}

/* bitwise_or := bitwise_xor ('|' bitwise_xor)* */
internal Expr *parse_expr_bitwise_or(Parser *p){
    Expr *left = parse_expr_bitwise_xor(p);
    while(parser_check(p, TK_Or)){
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *right = parse_expr_bitwise_xor(p);
        left = expr_binary(TK_Or, left, right, line, col);
    }
    return left;
}

/* bitwise_xor := bitwise_and ('^' bitwise_and)* */
internal Expr *parse_expr_bitwise_xor(Parser *p){
    Expr *left = parse_expr_bitwise_and(p);
    while(parser_check(p, TK_Xor)){
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *right = parse_expr_bitwise_and(p);
        left = expr_binary(TK_Xor, left, right, line, col);
    }
    return left;
}

/* bitwise_and := equality ('&' equality)* */
internal Expr *parse_expr_bitwise_and(Parser *p){
    Expr *left = parse_expr_equality(p);
    while(parser_check(p, TK_And)){
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *right = parse_expr_equality(p);
        left = expr_binary(TK_And, left, right, line, col);
    }
    return left;
}

/* equality := relational (('==' | '!=') relational)* */
internal Expr *parse_expr_equality(Parser *p){
    Expr *left = parse_expr_relational(p);
    while(parser_check(p, TK_Eq) || parser_check(p, TK_NotEq)){
        TokenKind op = p->lexer->token.kind;
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *right = parse_expr_relational(p);
        left = expr_binary(op, left, right, line, col);
    }
    return left;
}

/* relational := shift (('<' | '>' | '<=' | '>=') shift)* */
internal Expr *parse_expr_relational(Parser *p){
    Expr *left = parse_expr_shift(p);
    while(parser_check(p, TK_Less) || parser_check(p, TK_Grt) ||
          parser_check(p, TK_LessEq) || parser_check(p, TK_GrtEq)){
        TokenKind op = p->lexer->token.kind;
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *right = parse_expr_shift(p);
        left = expr_binary(op, left, right, line, col);
    }
    return left;
}

/* shift := additive (('<<' | '>>') additive)* */
internal Expr *parse_expr_shift(Parser *p){
    Expr *left = parse_expr_additive(p);
    while(parser_check(p, TK_ShiftL) || parser_check(p, TK_ShiftR)){
        TokenKind op = p->lexer->token.kind;
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *right = parse_expr_additive(p);
        left = expr_binary(op, left, right, line, col);
    }
    return left;
}

/* additive := multiplicative (('+' | '-') multiplicative)* */
internal Expr *parse_expr_additive(Parser *p){
    Expr *left = parse_expr_multiplicative(p);
    while(parser_check(p, TK_Plus) || parser_check(p, TK_Minus)){
        TokenKind op = p->lexer->token.kind;
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *right = parse_expr_multiplicative(p);
        left = expr_binary(op, left, right, line, col);
    }
    return left;
}

/* multiplicative := cast (('*' | '/' | '%') cast)* */
internal Expr *parse_expr_multiplicative(Parser *p){
    Expr *left = parse_expr_cast(p);
    while(parser_check(p, TK_Mul) || parser_check(p, TK_Div) || parser_check(p, TK_Mod)){
        TokenKind op = p->lexer->token.kind;
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;
        parser_advance(p);
        Expr *right = parse_expr_cast(p);
        left = expr_binary(op, left, right, line, col);
    }
    return left;
}

/*
 * cast := '(' type_name ')' cast
 *       | unary
 *
 * Ambiguity: '(' could start a parenthesised expression or a cast.
 * We peek: if the token after '(' is a type keyword, it's a cast.
 */
internal Expr *parse_expr_cast(Parser *p){
    if(parser_check(p, TK_OpenParen)){
        Token saved_token = p->lexer->token;
        Token saved_lookahead = p->lexer->lookahead;
        String saved_contents = p->lexer->contents;
        char saved_at0 = p->lexer->at[0];
        char saved_at1 = p->lexer->at[1];
        u32 saved_line = p->lexer->lastline;
        u32 saved_col = p->lexer->lastcolumn;
        LexerState saved_state = p->lexer->state;
        s32 saved_flag = p->lexer->flag;

        parser_advance(p);

        b32 looks_like_type = is_type_token(p);
        if(!looks_like_type && parser_check(p, TK_Name)){
            /* Could be a typedef'd type — for now, treat as expression. */
        }

        if(looks_like_type){
            TypeSpec *type = parse_full_type(p);
            if(parser_check(p, TK_CloseParen)){
                u32 line = saved_token.linenumber;
                u32 col  = saved_token.columnnumber;
                parser_advance(p);
                Expr *operand = parse_expr_cast(p);
                return expr_cast(type, operand, line, col);
            }
        }

        /* Not a cast — restore state and parse as expression. */
        p->lexer->token = saved_token;
        p->lexer->lookahead = saved_lookahead;
        p->lexer->contents = saved_contents;
        p->lexer->at[0] = saved_at0;
        p->lexer->at[1] = saved_at1;
        p->lexer->lastline = saved_line;
        p->lexer->lastcolumn = saved_col;
        p->lexer->state = saved_state;
        p->lexer->flag = saved_flag;
    }
    return parse_expr_unary(p);
}

/*
 * unary := '++' unary | '--' unary
 *        | '&' cast | '*' cast | '+' cast | '-' cast | '~' cast | '!' cast
 *        | 'sizeof' '(' type_name ')' | 'sizeof' unary
 *        | postfix
 */
internal Expr *parse_expr_unary(Parser *p){
    u32 line = p->lexer->token.linenumber;
    u32 col  = p->lexer->token.columnnumber;

    if(parser_check(p, TK_Inc)){
        parser_advance(p);
        return expr_unary(TK_Inc, parse_expr_unary(p), line, col);
    }
    if(parser_check(p, TK_Dec)){
        parser_advance(p);
        return expr_unary(TK_Dec, parse_expr_unary(p), line, col);
    }
    if(parser_check(p, TK_And)){
        parser_advance(p);
        return expr_unary(TK_And, parse_expr_cast(p), line, col);
    }
    if(parser_check(p, TK_Mul)){
        parser_advance(p);
        return expr_unary(TK_Mul, parse_expr_cast(p), line, col);
    }
    if(parser_check(p, TK_Plus)){
        parser_advance(p);
        return expr_unary(TK_Plus, parse_expr_cast(p), line, col);
    }
    if(parser_check(p, TK_Minus)){
        parser_advance(p);
        return expr_unary(TK_Minus, parse_expr_cast(p), line, col);
    }
    if(parser_check(p, TK_Tilde)){
        parser_advance(p);
        return expr_unary(TK_Tilde, parse_expr_cast(p), line, col);
    }
    if(parser_check(p, TK_Not)){
        parser_advance(p);
        return expr_unary(TK_Not, parse_expr_cast(p), line, col);
    }
    if(parser_check(p, TK_Sizeof)){
        parser_advance(p);
        if(parser_check(p, TK_OpenParen)){
            /* Try sizeof(type) first. */
            Token saved_token = p->lexer->token;
            Token saved_lookahead = p->lexer->lookahead;
            String saved_contents = p->lexer->contents;
            char saved_at0 = p->lexer->at[0];
            char saved_at1 = p->lexer->at[1];
            u32 saved_line = p->lexer->lastline;
            u32 saved_col = p->lexer->lastcolumn;
            LexerState saved_state = p->lexer->state;
            s32 saved_flag = p->lexer->flag;

            parser_advance(p);
            if(is_type_token(p)){
                TypeSpec *type = parse_full_type(p);
                parser_expect(p, TK_CloseParen, "expected ')' after sizeof(type)");
                return expr_sizeof_type(type, line, col);
            }

            /* Not a type — restore and parse as sizeof(expr). */
            p->lexer->token = saved_token;
            p->lexer->lookahead = saved_lookahead;
            p->lexer->contents = saved_contents;
            p->lexer->at[0] = saved_at0;
            p->lexer->at[1] = saved_at1;
            p->lexer->lastline = saved_line;
            p->lexer->lastcolumn = saved_col;
            p->lexer->state = saved_state;
            p->lexer->flag = saved_flag;
        }
        return expr_sizeof_expr(parse_expr_unary(p), line, col);
    }
    return parse_expr_postfix(p);
}

/*
 * postfix := primary (postfix_suffix)*
 * postfix_suffix := '(' args? ')' | '[' expr ']' | '.' name | '->' name | '++' | '--'
 */
internal Expr *parse_expr_postfix(Parser *p){
    Expr *left = parse_expr_primary(p);

    for(;;){
        u32 line = p->lexer->token.linenumber;
        u32 col  = p->lexer->token.columnnumber;

        if(parser_check(p, TK_OpenParen)){
            parser_advance(p);
            Expr **args = 0;
            s32 num_args = 0;
            if(!parser_check(p, TK_CloseParen)){
                Expr *arg = parse_expr_assign(p);
                DynamicArray__fit(args, 1);
                args[darr_len(args)] = arg;
                DynamicArray__header(args)->len += 1;
                num_args++;
                while(parser_match(p, TK_Comma)){
                    arg = parse_expr_assign(p);
                    DynamicArray__fit(args, 1);
                    args[darr_len(args)] = arg;
                    DynamicArray__header(args)->len += 1;
                    num_args++;
                }
            }
            parser_expect(p, TK_CloseParen, "expected ')' after function arguments");
            left = expr_call(left, args, num_args, line, col);
        }
        else if(parser_check(p, TK_OpenBracket)){
            parser_advance(p);
            Expr *idx = parse_expr(p);
            parser_expect(p, TK_CloseBracket, "expected ']' after array subscript");
            left = expr_index(left, idx, line, col);
        }
        else if(parser_check(p, TK_Access)){
            parser_advance(p);
            if(!parser_check(p, TK_Name)){
                parse_error(p, "expected field name after '.'");
                break;
            }
            String fname = p->lexer->token.lexeme;
            parser_advance(p);
            left = expr_field(left, fname, line, col);
        }
        else if(parser_check(p, TK_Arrow)){
            parser_advance(p);
            if(!parser_check(p, TK_Name)){
                parse_error(p, "expected field name after '->'");
                break;
            }
            String fname = p->lexer->token.lexeme;
            parser_advance(p);
            left = expr_field_ptr(left, fname, line, col);
        }
        else if(parser_check(p, TK_Inc)){
            parser_advance(p);
            left = expr_postfix(TK_Inc, left, line, col);
        }
        else if(parser_check(p, TK_Dec)){
            parser_advance(p);
            left = expr_postfix(TK_Dec, left, line, col);
        }
        else{
            break;
        }
    }
    return left;
}

/* primary := int | float | string | char | name | '(' expr ')' */
internal Expr *parse_expr_primary(Parser *p){
    u32 line = p->lexer->token.linenumber;
    u32 col  = p->lexer->token.columnnumber;

    if(parser_check(p, TK_IntNum)){
        u64 val = p->lexer->token.u.U64;
        parser_advance(p);
        /* Skip optional integer suffix tokens. */
        while(p->lexer->token.kind >= TK_IntSufU && p->lexer->token.kind <= TK_IntSufLL){
            parser_advance(p);
        }
        return expr_int(val, line, col);
    }
    if(parser_check(p, TK_FloatNum)){
        f64 val = p->lexer->token.u.F64;
        parser_advance(p);
        /* Skip optional float suffix tokens. */
        if(p->lexer->token.kind == TK_FloatSufL || p->lexer->token.kind == TK_FloatSufF){
            parser_advance(p);
        }
        return expr_float(val, line, col);
    }
    if(parser_check(p, TK_String)){
        String val = p->lexer->token.lexeme;
        parser_advance(p);
        return expr_str(val, line, col);
    }
    if(parser_check(p, TK_Name)){
        String name = p->lexer->token.lexeme;
        parser_advance(p);
        return expr_name(name, line, col);
    }
    if(parser_check(p, TK_OpenParen)){
        parser_advance(p);
        Expr *inner = parse_expr(p);
        parser_expect(p, TK_CloseParen, "expected ')' after expression");
        return expr_paren(inner, line, col);
    }

    parse_error(p, "expected expression");
    return expr_int(0, line, col);
}

/* ======================================================================
 * Statement parsing
 * ====================================================================== */

internal Stmt *parse_stmt(Parser *p){
    u32 line = p->lexer->token.linenumber;
    u32 col  = p->lexer->token.columnnumber;

    if(parser_check(p, TK_OpenBrace)){
        return parse_stmt_block(p);
    }
    if(parser_check(p, TK_If)){
        parser_advance(p);
        parser_expect(p, TK_OpenParen, "expected '(' after 'if'");
        Expr *cond = parse_expr(p);
        parser_expect(p, TK_CloseParen, "expected ')' after if condition");
        Stmt *then_body = parse_stmt(p);
        Stmt *else_body = 0;
        if(parser_match(p, TK_Else)){
            else_body = parse_stmt(p);
        }
        return stmt_if(cond, then_body, else_body, line, col);
    }
    if(parser_check(p, TK_While)){
        parser_advance(p);
        parser_expect(p, TK_OpenParen, "expected '(' after 'while'");
        Expr *cond = parse_expr(p);
        parser_expect(p, TK_CloseParen, "expected ')' after while condition");
        Stmt *body = parse_stmt(p);
        return stmt_while(cond, body, line, col);
    }
    if(parser_check(p, TK_Do)){
        parser_advance(p);
        Stmt *body = parse_stmt(p);
        parser_expect(p, TK_While, "expected 'while' after do body");
        parser_expect(p, TK_OpenParen, "expected '(' after 'while'");
        Expr *cond = parse_expr(p);
        parser_expect(p, TK_CloseParen, "expected ')' after do-while condition");
        parser_expect(p, TK_Semi, "expected ';' after do-while");
        return stmt_do_while(body, cond, line, col);
    }
    if(parser_check(p, TK_For)){
        parser_advance(p);
        parser_expect(p, TK_OpenParen, "expected '(' after 'for'");

        Stmt *init = 0;
        if(!parser_check(p, TK_Semi)){
            if(is_type_token(p)){
                ASTDecl *d = parse_decl(p);
                init = stmt_decl(d, line, col);
            } else {
                Expr *e = parse_expr(p);
                init = stmt_expr(e, line, col);
                parser_expect(p, TK_Semi, "expected ';' in for initializer");
            }
        } else {
            parser_advance(p);
        }

        Expr *cond = 0;
        if(!parser_check(p, TK_Semi)){
            cond = parse_expr(p);
        }
        parser_expect(p, TK_Semi, "expected ';' after for condition");

        Expr *post = 0;
        if(!parser_check(p, TK_CloseParen)){
            post = parse_expr(p);
        }
        parser_expect(p, TK_CloseParen, "expected ')' after for clauses");

        Stmt *body = parse_stmt(p);
        return stmt_for(init, cond, post, body, line, col);
    }
    if(parser_check(p, TK_Switch)){
        parser_advance(p);
        parser_expect(p, TK_OpenParen, "expected '(' after 'switch'");
        Expr *sw_expr = parse_expr(p);
        parser_expect(p, TK_CloseParen, "expected ')' after switch expression");
        parser_expect(p, TK_OpenBrace, "expected '{' for switch body");

        SwitchCase *cases = 0;
        s32 num_cases = 0;

        while(!parser_check(p, TK_CloseBrace) && !parser_check(p, TK_Eoi)){
            SwitchCase sc = {0};
            if(parser_check(p, TK_Case)){
                parser_advance(p);
                sc.value = parse_expr_ternary(p);
                sc.is_default = 0;
            } else if(parser_check(p, TK_Default)){
                parser_advance(p);
                sc.is_default = 1;
            } else {
                parse_error(p, "expected 'case' or 'default' in switch");
                parser_synchronize(p);
                continue;
            }
            parser_expect(p, TK_Colon, "expected ':' after case/default");

            Stmt **case_stmts = 0;
            s32 case_num_stmts = 0;
            while(!parser_check(p, TK_Case) && !parser_check(p, TK_Default) &&
                  !parser_check(p, TK_CloseBrace) && !parser_check(p, TK_Eoi)){
                Stmt *st = parse_stmt(p);
                DynamicArray__fit(case_stmts, 1);
                case_stmts[darr_len(case_stmts)] = st;
                DynamicArray__header(case_stmts)->len += 1;
                case_num_stmts++;
            }
            sc.stmts = case_stmts;
            sc.num_stmts = case_num_stmts;

            DynamicArray__fit(cases, 1);
            cases[darr_len(cases)] = sc;
            DynamicArray__header(cases)->len += 1;
            num_cases++;
        }
        parser_expect(p, TK_CloseBrace, "expected '}' after switch body");
        return stmt_switch(sw_expr, cases, num_cases, line, col);
    }
    if(parser_check(p, TK_Return)){
        parser_advance(p);
        Expr *val = 0;
        if(!parser_check(p, TK_Semi)){
            val = parse_expr(p);
        }
        parser_expect(p, TK_Semi, "expected ';' after return");
        return stmt_return(val, line, col);
    }
    if(parser_check(p, TK_Break)){
        parser_advance(p);
        parser_expect(p, TK_Semi, "expected ';' after break");
        return stmt_break(line, col);
    }
    if(parser_check(p, TK_Continue)){
        parser_advance(p);
        parser_expect(p, TK_Semi, "expected ';' after continue");
        return stmt_continue(line, col);
    }
    if(parser_check(p, TK_Goto)){
        parser_advance(p);
        if(!parser_check(p, TK_Name)){
            parse_error(p, "expected label name after 'goto'");
            return stmt_goto((String){0}, line, col);
        }
        String label = p->lexer->token.lexeme;
        parser_advance(p);
        parser_expect(p, TK_Semi, "expected ';' after goto");
        return stmt_goto(label, line, col);
    }

    /* label: stmt — a name followed by ':' */
    if(parser_check(p, TK_Name)){
        look_ahead(p->lexer);
        if(is_token(p->lexer->lookahead, TK_Colon)){
            String label = p->lexer->token.lexeme;
            parser_advance(p);
            parser_advance(p);
            Stmt *inner = parse_stmt(p);
            return stmt_label(label, inner, line, col);
        }
    }

    /* Declaration as statement (inside a block). */
    if(is_type_token(p)){
        ASTDecl *d = parse_decl(p);
        return stmt_decl(d, line, col);
    }

    /* Expression statement. */
    if(parser_check(p, TK_Semi)){
        parser_advance(p);
        return stmt_expr(0, line, col);
    }
    {
        Expr *e = parse_expr(p);
        parser_expect(p, TK_Semi, "expected ';' after expression");
        return stmt_expr(e, line, col);
    }
}

/* compound_stmt := '{' stmt* '}' */
internal Stmt *parse_stmt_block(Parser *p){
    u32 line = p->lexer->token.linenumber;
    u32 col  = p->lexer->token.columnnumber;

    parser_expect(p, TK_OpenBrace, "expected '{'");

    Stmt **stmts = 0;
    s32 num_stmts = 0;

    while(!parser_check(p, TK_CloseBrace) && !parser_check(p, TK_Eoi)){
        Stmt *s = parse_stmt(p);
        DynamicArray__fit(stmts, 1);
        stmts[darr_len(stmts)] = s;
        DynamicArray__header(stmts)->len += 1;
        num_stmts++;
    }

    parser_expect(p, TK_CloseBrace, "expected '}'");
    return stmt_block(stmts, num_stmts, line, col);
}

/* ======================================================================
 * Declaration parsing
 * ====================================================================== */

internal StorageClass parse_storage_class(Parser *p){
    if(parser_match(p, TK_Static))   return STORAGE_STATIC;
    if(parser_match(p, TK_Extern))   return STORAGE_EXTERN;
    if(parser_match(p, TK_Auto))     return STORAGE_AUTO;
    if(parser_match(p, TK_Register)) return STORAGE_REGISTER;
    return STORAGE_NONE;
}

/*
 * parse_decl — top-level declaration parser.
 *
 * Handles: typedef, struct, union, enum declarations, and the ambiguity
 * between function definitions/declarations and variable declarations.
 */
internal ASTDecl *parse_decl(Parser *p){
    u32 line = p->lexer->token.linenumber;
    u32 col  = p->lexer->token.columnnumber;

    /* typedef */
    if(parser_check(p, TK_Typedef)){
        return parse_decl_typedef(p);
    }

    StorageClass sc = parse_storage_class(p);

    /* Standalone struct/union/enum declarations (without a variable). */
    if(parser_check(p, TK_Struct) || parser_check(p, TK_Union) || parser_check(p, TK_Enum)){
        /* Parse the type first, then decide. */
        TypeSpec *type = parse_base_type(p);

        /* If followed by a name, it could be a variable or function. */
        if(parser_check(p, TK_Name)){
            String name = p->lexer->token.lexeme;
            parser_advance(p);

            /* Pointer decorations on the variable/parameter. */
            while(parser_check(p, TK_Mul)){
                parser_advance(p);
                type = typespec_ptr(type);
            }
            if(parser_check(p, TK_Name)){
                name = p->lexer->token.lexeme;
                parser_advance(p);
            }

            if(parser_check(p, TK_OpenParen)){
                return parse_decl_func_rest(p, type, name, sc, line, col);
            }

            /* Variable declaration. */
            Expr *init = 0;
            if(parser_match(p, TK_Asgn)){
                init = parse_expr_assign(p);
            }
            parser_expect(p, TK_Semi, "expected ';' after variable declaration");
            return decl_var(type, name, init, sc, line, col);
        }

        /* Standalone struct/union/enum declaration: "struct Foo { ... };" */
        if(parser_check(p, TK_Semi)) parser_advance(p);

        if(type->kind == TYPESPEC_STRUCT){
            return decl_struct(type->aggregate.name, type->aggregate.fields,
                               type->aggregate.num_fields, line, col);
        }
        if(type->kind == TYPESPEC_UNION){
            return decl_union(type->aggregate.name, type->aggregate.fields,
                              type->aggregate.num_fields, line, col);
        }
        if(type->kind == TYPESPEC_ENUM){
            return decl_enum(type->enumeration.name, type->enumeration.items,
                             type->enumeration.num_items, line, col);
        }
    }

    /* Regular type + name. */
    TypeSpec *type = parse_full_type(p);

    /* Abstract (no name): forward declaration edge case. */
    if(parser_check(p, TK_Semi)){
        parser_advance(p);
        String empty = {0};
        return decl_var(type, empty, 0, sc, line, col);
    }

    if(!parser_check(p, TK_Name)){
        parse_error(p, "expected declarator name");
        parser_synchronize(p);
        String empty = {0};
        return decl_var(type, empty, 0, sc, line, col);
    }

    String name = p->lexer->token.lexeme;
    parser_advance(p);

    /* Array declarator: name '[' size? ']' */
    if(parser_check(p, TK_OpenBracket)){
        parser_advance(p);
        Expr *size = 0;
        if(!parser_check(p, TK_CloseBracket)){
            size = parse_expr_assign(p);
        }
        parser_expect(p, TK_CloseBracket, "expected ']' after array size");
        type = typespec_array(type, size);
        Expr *init = 0;
        if(parser_match(p, TK_Asgn)){
            init = parse_expr_assign(p);
        }
        parser_expect(p, TK_Semi, "expected ';' after array declaration");
        return decl_var(type, name, init, sc, line, col);
    }

    /* Function declaration / definition. */
    if(parser_check(p, TK_OpenParen)){
        return parse_decl_func_rest(p, type, name, sc, line, col);
    }

    /* Variable declaration, possibly with initializer. */
    Expr *init = 0;
    if(parser_match(p, TK_Asgn)){
        init = parse_expr_assign(p);
    }
    parser_expect(p, TK_Semi, "expected ';' after variable declaration");
    return decl_var(type, name, init, sc, line, col);
}

/*
 * Parse the rest of a function after we've consumed: type name
 * Now we expect: '(' params? ')' (block | ';')
 */
internal ASTDecl *parse_decl_func_rest(Parser *p, TypeSpec *ret_type, String name,
                                        StorageClass sc, u32 line, u32 col){
    parser_expect(p, TK_OpenParen, "expected '(' for function parameters");

    FuncParam *params = 0;
    s32 num_params = 0;
    b32 has_varargs = 0;

    if(!parser_check(p, TK_CloseParen)){
        /* Check for f(void) */
        if(parser_check(p, TK_Void)){
            look_ahead(p->lexer);
            if(is_token(p->lexer->lookahead, TK_CloseParen)){
                parser_advance(p);
                goto done_params;
            }
        }

        for(;;){
            if(parser_check(p, TK_Dots)){
                has_varargs = 1;
                parser_advance(p);
                break;
            }
            TypeSpec *ptype = parse_full_type(p);
            String pname = {0};
            if(parser_check(p, TK_Name)){
                pname = p->lexer->token.lexeme;
                parser_advance(p);
            }
            /* Handle array parameters: int arr[] */
            if(parser_check(p, TK_OpenBracket)){
                parser_advance(p);
                Expr *arr_size = 0;
                if(!parser_check(p, TK_CloseBracket)){
                    arr_size = parse_expr_assign(p);
                }
                parser_expect(p, TK_CloseBracket, "expected ']'");
                ptype = typespec_array(ptype, arr_size);
            }

            FuncParam fp = {0};
            fp.type = ptype;
            fp.name = pname;
            DynamicArray__fit(params, 1);
            params[darr_len(params)] = fp;
            DynamicArray__header(params)->len += 1;
            num_params++;

            if(!parser_match(p, TK_Comma)) break;
        }
    }

done_params:
    parser_expect(p, TK_CloseParen, "expected ')' after function parameters");

    Stmt *body = 0;
    if(parser_check(p, TK_OpenBrace)){
        body = parse_stmt_block(p);
    } else {
        parser_expect(p, TK_Semi, "expected ';' or '{' after function declaration");
    }

    return decl_func(ret_type, name, params, num_params, has_varargs, body, sc, line, col);
}

internal ASTDecl *parse_decl_enum(Parser *p){
    u32 line = p->lexer->token.linenumber;
    u32 col  = p->lexer->token.columnnumber;
    parser_expect(p, TK_Enum, "expected 'enum'");

    String name = {0};
    if(parser_check(p, TK_Name)){
        name = p->lexer->token.lexeme;
        parser_advance(p);
    }

    EnumItem *items = 0;
    s32 num_items = 0;

    if(parser_check(p, TK_OpenBrace)){
        parser_advance(p);
        while(!parser_check(p, TK_CloseBrace) && !parser_check(p, TK_Eoi)){
            String item_name = {0};
            if(parser_check(p, TK_Name)){
                item_name = p->lexer->token.lexeme;
                parser_advance(p);
            }
            Expr *val = 0;
            if(parser_match(p, TK_Asgn)){
                val = parse_expr_ternary(p);
            }
            EnumItem ei = {0};
            ei.name = item_name;
            ei.value = val;
            DynamicArray__fit(items, 1);
            items[darr_len(items)] = ei;
            DynamicArray__header(items)->len += 1;
            num_items++;
            if(!parser_match(p, TK_Comma)) break;
        }
        parser_expect(p, TK_CloseBrace, "expected '}' after enum body");
    }

    if(parser_check(p, TK_Semi)) parser_advance(p);
    return decl_enum(name, items, num_items, line, col);
}

internal ASTDecl *parse_decl_aggregate(Parser *p, b32 is_struct){
    u32 line = p->lexer->token.linenumber;
    u32 col  = p->lexer->token.columnnumber;
    parser_advance(p);

    String name = {0};
    if(parser_check(p, TK_Name)){
        name = p->lexer->token.lexeme;
        parser_advance(p);
    }

    AggregateField *fields = 0;
    s32 num_fields = 0;

    if(parser_check(p, TK_OpenBrace)){
        parser_advance(p);
        while(!parser_check(p, TK_CloseBrace) && !parser_check(p, TK_Eoi)){
            TypeSpec *ft = parse_full_type(p);
            String fn = {0};
            if(parser_check(p, TK_Name)){
                fn = p->lexer->token.lexeme;
                parser_advance(p);
            }
            Expr *bitfield = 0;
            if(parser_match(p, TK_Colon)){
                bitfield = parse_expr_ternary(p);
            }
            AggregateField af = {0};
            af.type = ft;
            af.name = fn;
            af.bitfield = bitfield;
            DynamicArray__fit(fields, 1);
            fields[darr_len(fields)] = af;
            DynamicArray__header(fields)->len += 1;
            num_fields++;
            parser_expect(p, TK_Semi, "expected ';' after struct/union field");
        }
        parser_expect(p, TK_CloseBrace, "expected '}' after struct/union body");
    }

    if(parser_check(p, TK_Semi)) parser_advance(p);

    if(is_struct) return decl_struct(name, fields, num_fields, line, col);
    else          return decl_union(name, fields, num_fields, line, col);
}

internal ASTDecl *parse_decl_struct(Parser *p){
    return parse_decl_aggregate(p, 1);
}

internal ASTDecl *parse_decl_union(Parser *p){
    return parse_decl_aggregate(p, 0);
}

internal ASTDecl *parse_decl_typedef(Parser *p){
    u32 line = p->lexer->token.linenumber;
    u32 col  = p->lexer->token.columnnumber;
    parser_expect(p, TK_Typedef, "expected 'typedef'");

    TypeSpec *type = parse_full_type(p);

    String name = {0};
    if(parser_check(p, TK_Name)){
        name = p->lexer->token.lexeme;
        parser_advance(p);
    }

    parser_expect(p, TK_Semi, "expected ';' after typedef");
    return decl_typedef(type, name, line, col);
}

/* ======================================================================
 * Top-level: parse a translation unit
 * ====================================================================== */

internal ASTDecl **parse_translation_unit(Parser *p){
    ASTDecl **decls = 0;

    while(!parser_check(p, TK_Eoi)){
        if(p->panic_mode) parser_synchronize(p);
        if(parser_check(p, TK_Eoi)) break;

        ASTDecl *d = parse_decl(p);
        DynamicArray__fit(decls, 1);
        decls[darr_len(decls)] = d;
        DynamicArray__header(decls)->len += 1;
    }

    return decls;
}

/* ======================================================================
 * Legacy expression evaluator (kept for existing test_parse_expr tests)
 * ====================================================================== */

internal s64 expr0(Lexer *lexer){
    s64 result = 0;
    nexttoken(lexer);
    result = expr1(lexer);
    if(!is_token(lexer->token, TK_Semi)){
        /* error handling */
    }
    return(result);
}

internal s64 expr1(Lexer *lexer){
    s64 result = expr2(lexer);
    while(is_token(lexer->token, TK_Plus) ||
          is_token(lexer->token, TK_Minus) ||
          is_token(lexer->token, TK_And) ||
          is_token(lexer->token, TK_Or) ||
          is_token(lexer->token, TK_Xor) ||
          is_token(lexer->token, TK_ShiftL) ||
          is_token(lexer->token, TK_ShiftR)){
        s32 op = (lexer->token.kind);
        nexttoken(lexer);
        switch(op){
            case(TK_Plus):{ result += expr2(lexer); }break;
            case(TK_Minus):{ result -= expr2(lexer); }break;
            case(TK_And):{ result &= expr2(lexer); }break;
            case(TK_Or):{ result |= expr2(lexer); }break;
            case(TK_Xor):{ result ^= expr2(lexer); }break;
            case(TK_ShiftL):{ result <<= expr2(lexer); }break;
            case(TK_ShiftR):{ result >>= expr2(lexer); }break;
        }
    }
    return(result);
}

internal s64 expr2(Lexer *lexer){
    s64 result = expr3(lexer);
    while(is_token(lexer->token, TK_Mul) ||
          is_token(lexer->token, TK_Div) ||
          is_token(lexer->token, TK_Mod)){
        s32 op = lexer->token.kind;
        nexttoken(lexer);
        switch(op){
            case(TK_Mul):{ result *= expr3(lexer); }break;
            case(TK_Div):{
                s64 tmp = expr3(lexer);
                if(tmp == 0){ /* division by zero */ }
                else{ result /= tmp; }
            }break;
            case(TK_Mod):{ result %= expr3(lexer); }break;
        }
    }
    return(result);
}

internal s64 expr3(Lexer *lexer){
    s64 result = 0;
    if(is_token(lexer->token, TK_Minus)){
        nexttoken(lexer);
        result = -expr4(lexer);
    }
    else if(is_token(lexer->token, TK_Plus)){
        nexttoken(lexer);
        result = expr4(lexer);
    }
    else{
        result = expr4(lexer);
    }
    return(result);
}

internal s64 expr4(Lexer *lexer){
    s64 result = 0;
    if(is_token(lexer->token, TK_OpenParen)){
        nexttoken(lexer);
        result = expr1(lexer);
        if(is_token(lexer->token, TK_CloseParen)){
            nexttoken(lexer);
        }
    }
    else if(is_token(lexer->token, TK_IntNum)){
        result = lexer->token.u.U64;
        nexttoken(lexer);
    }
    else if(is_token(lexer->token, TK_FloatNum)){
        result = (s64)lexer->token.u.F64;
        nexttoken(lexer);
    }
    else if(is_token(lexer->token, TK_Name)){
        /* identifiers not supported in constant evaluator */
        nexttoken(lexer);
    }
    return(result);
}
