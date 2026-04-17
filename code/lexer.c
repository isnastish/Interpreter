

/*
 * Aleksey Yevtushenko
 * February 13, 2022
 */

global String *intern_table;

/* C keywords. */
global String auto_keyword;
global String break_keyword;
global String case_keyword;
global String char_keyword;
global String const_keyword;
global String continue_keyword;
global String default_keyword;
global String do_keyword;
global String double_keyword;
global String else_keyword;
global String enum_keyword;
global String extern_keyword;
global String float_keyword;
global String for_keyword;
global String goto_keyword; 
global String if_keyword;
global String int_keyword;
global String long_keyword;
global String register_keyword;
global String return_keyword;
global String short_keyword;
global String signed_keyword;
global String sizeof_keyword;
global String static_keyword;
global String struct_keyword;
global String switch_keyword;
global String typedef_keyword;
global String union_keyword;
global String unsigned_keyword;
global String void_keyword;
global String volatile_keyword;
global String while_keyword;

/* C preprocessor directives. */
global String dir_if;
global String dir_ifdef;
global String dir_ifndef;
global String dir_elif;
global String dir_else;
global String dir_endif;
global String dir_include;
global String dir_define;
global String dir_undef;
global String dir_line;
global String dir_error;
global String dir_pragma;

/* Intern all C keywords and all allowed preprocessor directives
   before any call to gettoken() function. */
internal void intern_reserved(String **intern_table){
    auto_keyword = string_intern(intern_table, "auto");
    break_keyword = string_intern(intern_table, "break");
    case_keyword = string_intern(intern_table, "case");
    char_keyword = string_intern(intern_table, "char");
    const_keyword = string_intern(intern_table, "const");
    continue_keyword = string_intern(intern_table, "continue");
    default_keyword = string_intern(intern_table, "default");
    do_keyword = string_intern(intern_table, "do");
    double_keyword = string_intern(intern_table, "double");
    else_keyword = string_intern(intern_table, "else");
    enum_keyword = string_intern(intern_table, "enum");
    extern_keyword = string_intern(intern_table, "extern");
    float_keyword = string_intern(intern_table, "float");
    for_keyword = string_intern(intern_table, "for");
    goto_keyword = string_intern(intern_table, "goto");
    if_keyword = string_intern(intern_table, "if");
    int_keyword = string_intern(intern_table, "int");
    long_keyword = string_intern(intern_table, "long");
    register_keyword = string_intern(intern_table, "register");
    return_keyword = string_intern(intern_table, "return");
    short_keyword = string_intern(intern_table, "short");
    signed_keyword = string_intern(intern_table, "signed");
    sizeof_keyword = string_intern(intern_table, "sizeof");
    static_keyword = string_intern(intern_table, "static");
    struct_keyword = string_intern(intern_table, "struct");
    switch_keyword = string_intern(intern_table, "switch");
    typedef_keyword = string_intern(intern_table, "typedef");
    union_keyword = string_intern(intern_table, "union");
    unsigned_keyword = string_intern(intern_table, "unsigned");
    void_keyword = string_intern(intern_table, "void");
    volatile_keyword = string_intern(intern_table, "volatile");
    while_keyword = string_intern(intern_table, "while");
    
    dir_if = if_keyword; // Already interned.
    dir_ifdef = string_intern(intern_table, "ifdef");
    dir_ifndef = string_intern(intern_table, "ifndef");
    dir_elif = string_intern(intern_table, "elif");
    dir_else = else_keyword; // Already interned.
    dir_endif = string_intern(intern_table, "endif");
    dir_include = string_intern(intern_table, "include");
    dir_define = string_intern(intern_table, "define");
    dir_undef = string_intern(intern_table, "undef");
    dir_line = string_intern(intern_table, "line");
    dir_error = string_intern(intern_table, "error");
    dir_pragma = string_intern(intern_table, "pragma");
}

internal b32 is_keyword(Token *token){
    char *intern = (token->u.intern.data);
    
    if(intern == auto_keyword.data){ token->kind = TK_Auto; return(1); }
    else if(intern == break_keyword.data){ token->kind = TK_Break; return(1); }
    else if(intern == case_keyword.data){ token->kind = TK_Case; return(1); }
    else if(intern == char_keyword.data){ token->kind = TK_Char; return(1); }
    else if(intern == const_keyword.data){ token->kind = TK_Const; return(1); }
    else if(intern == continue_keyword.data){ token->kind = TK_Continue; return(1); }
    else if(intern == default_keyword.data){ token->kind = TK_Default; return(1); }
    else if(intern == double_keyword.data){ token->kind = TK_Double; return(1); }
    else if(intern == do_keyword.data){ token->kind = TK_Do; return(1); }
    else if(intern == else_keyword.data){ token->kind = TK_Else; return(1); }
    else if(intern == enum_keyword.data){ token->kind = TK_Enum; return(1); }
    else if(intern == extern_keyword.data){ token->kind = TK_Extern; return(1); }
    else if(intern == float_keyword.data){ token->kind = TK_Float; return(1); }
    else if(intern == for_keyword.data){ token->kind = TK_For; return(1); }
    else if(intern == goto_keyword.data){ token->kind = TK_Goto; return(1); }
    else if(intern == if_keyword.data){ token->kind = TK_If; return(1); }
    else if(intern == int_keyword.data){ token->kind = TK_Int; return(1); }
    else if(intern == long_keyword.data){ token->kind = TK_Long; return(1); }
    else if(intern == register_keyword.data){ token->kind = TK_Register; return(1); }
    else if(intern == return_keyword.data){ token->kind = TK_Return; return(1); }
    else if(intern == short_keyword.data){ token->kind = TK_Short; return(1); }
    else if(intern == signed_keyword.data){ token->kind = TK_Signed; return(1); }
    else if(intern == sizeof_keyword.data){ token->kind = TK_Sizeof; return(1); }
    else if(intern == static_keyword.data){ token->kind = TK_Static; return(1); }
    else if(intern == struct_keyword.data){ token->kind = TK_Struct; return(1); }
    else if(intern == switch_keyword.data){ token->kind = TK_Switch; return(1); }
    else if(intern == typedef_keyword.data){ token->kind = TK_Typedef; return(1); }
    else if(intern == union_keyword.data){ token->kind = TK_Union; return(1); }
    else if(intern == unsigned_keyword.data){ token->kind = TK_Unsigned; return(1); }
    else if(intern == void_keyword.data){ token->kind = TK_Void; return(1); }
    else if(intern == volatile_keyword.data){ token->kind = TK_Volatile; return(1); }
    else if(intern == while_keyword.data){ token->kind = TK_While; return(1); }
    
    return(0);
}

internal b32 is_ppdir(Token *token){
    char *intern = (token->u.intern.data);
    
    if(intern == dir_if.data){ token->kind = TK_If_dir; return(1); }
    else if(intern == dir_ifdef.data){ token->kind = TK_Ifdef_dir; return(1); }
    else if(intern == dir_ifndef.data){ token->kind = TK_Ifndef_dir; return(1); }
    else if(intern == dir_elif.data){ token->kind = TK_Elif_dir; return(1); }
    else if(intern == dir_else.data){ token->kind = TK_Else_dir; return(1); }
    else if(intern == dir_endif.data){ token->kind = TK_Endif_dir; return(1); }
    else if(intern == dir_include.data){ token->kind = TK_Include_dir; return(1); }
    else if(intern == dir_define.data){ token->kind = TK_Define_dir; return(1); }
    else if(intern == dir_undef.data){ token->kind = TK_Undef_dir; return(1); }
    else if(intern == dir_line.data){ token->kind = TK_Line_dir; return(1); }
    else if(intern == dir_error.data){ token->kind = TK_Error_dir; return(1); }
    else if(intern == dir_pragma.data){ token->kind = TK_Pragma_dir; return(1); }
    
    //
    // NOTE: What about TK_defined?
    //
    
    return(0);
}

internal void refill(Lexer *lexer){
    if(lexer->contents.count == 0){
        lexer->at[0] = 0;
        lexer->at[1] = 0;
    }
    else if(lexer->contents.count == 1){
        lexer->at[0] = lexer->contents.data[0];
        lexer->at[1] = 0;
    }
    else{
        lexer->at[0] = lexer->contents.data[0];
        lexer->at[1] = lexer->contents.data[1];
    }
}

internal void advance_chars(Lexer *lexer, u32 count){
    lexer->lastcolumn += count;
    advance(&lexer->contents, count);
    refill(lexer);
}

internal u64 read_integer(Lexer *lexer, Token *token){
    u64 result = 0;
    
    if((lexer->at[0] == '0') && oneof(lexer->at[1], "xX")){ // hexidecimal.
        token->skind = TSK_Hex;
        advance_chars(lexer, 2);
        while(is_hex(lexer->at[0])){
            u32 digit = hex_val(lexer->at[0]);
            if(result > ((U64_MAX - digit) / 16)){
                lexer->error = LError_U64Overflow;
                break;
            }
            result = ((result * 16) + digit);
            advance_chars(lexer, 1);
        }
        if(is_alpha(lexer->at[0]) && (lexer->error != LError_U64Overflow)){
            lexer->error = LError_WrongHexSym;
        }
    }
    else if((lexer->at[0] == '0') && oneof(lexer->at[1], "bB")){ // binary.
        token->skind = TSK_Bin;
        advance_chars(lexer, 2);
        while(is_bin(lexer->at[0])){
            u32 digit = (lexer->at[0] - '0');
            if(result > ((U64_MAX - digit) / 2)){
                lexer->error = LError_U64Overflow;
                break;
            }
            result = ((result * 2) + digit); 
            advance_chars(lexer, 1);
        }
        if(is_digit(lexer->at[0]) && (lexer->error != LError_U64Overflow)){
            lexer->error = LError_WrongBinSym;
        }
    }
    else if((lexer->at[0] == '0') && is_digit(lexer->at[1])){ // octal.
        token->skind = TSK_Oct;
        while(is_oct(lexer->at[0])){
            u32 digit = (lexer->at[0] - '0');
            if(result > ((U64_MAX - digit) / 8)){
                lexer->error = LError_U64Overflow;
                break;
            }
            result = ((result * 8) + digit);
            advance_chars(lexer, 1);
        }
        if(is_digit(lexer->at[0]) && (lexer->error != LError_U64Overflow)){
            lexer->error = LError_WrongOctSym;
        }
    }
    else{ // decimal.
        while(is_digit(lexer->at[0])){
            u32 digit = (lexer->at[0] - '0');
            if(result > ((U64_MAX - digit) / 10)){
                lexer->error = LError_U64Overflow;
                break;
            }
            result = ((result * 10) + digit);
            advance_chars(lexer, 1);
        }
    }
    
    if(lexer->error != LError_Success){
        while(is_alnum(lexer->at[0])){
            advance_chars(lexer, 1);
        }
        result = 0;
    }
    
    return(result);
}

// TODO: Read hex floats.
internal f64 read_float(Lexer *lexer){
    f64 result = 0.0;
    
    char *float_start = (lexer->contents.data);
    while(is_digit(lexer->at[0])){
        advance_chars(lexer, 1);
    }
    if(lexer->at[0] == '.'){
        advance_chars(lexer, 1);
        while(is_digit(lexer->at[0])){
            advance_chars(lexer, 1);
        }
    }
    if(oneof(lexer->at[0], "eE")){
        advance_chars(lexer, 1);
        if(oneof(lexer->at[0], "-+")){
            advance_chars(lexer, 1);
        }
        if(!is_digit(lexer->at[0])){
            lexer->error = LError_ExpDigit;
        }
        while(is_digit(lexer->at[0])){
            advance_chars(lexer, 1);
        }
    }
    if(lexer->error == LError_Success){
        result=strtod(float_start, 0);
        if((result == HUGE_VAL) || (result == -HUGE_VAL)){
            lexer->error = LError_F64Overflow;
            result = 0;
        }
    }
    
    return(result);
}

// NOTE: The first quote has already been consumed.
internal u64 read_char(Lexer *lexer){
    u64 result = 0;
    
    if(lexer->at[0] == '\''){
        lexer->error = LError_EmptyCharLit;
    }
    else if(is_newline(lexer->at[0])){
        lexer->error=LError_NLInConst;
        if(((lexer->at[0] == '\r') && (lexer->at[1] == '\n')) ||
           ((lexer->at[0] == '\n') && (lexer->at[1] == '\r'))){
            advance_chars(lexer, 1);
        }
        advance_chars(lexer, 1);
    }
    else if(lexer->at[0] == '\\'){
        char c = lexer->at[1];
        advance_chars(lexer, 2);
        switch(c){
            case('0'): {result = '\0';}break; // NOTE: can be oct escape sequence.
            case('n'): {result = '\n';}break;
            case('r'): {result = '\r';}break;
            case('f'): {result = '\f';}break;
            case('t'): {result = '\t';}break;
            case('v'): {result = '\v';}break;
            case('b'): {result = '\b';}break;
            case('a'): {result = '\a';}break;
            case('\\'):{result = '\\';}break;
            case('?'): {result = '\?';}break;
            case('\''):{result = '\'';}break;
            case('\"'):{result = '\"';}break;
            default:{
                lexer->error = LError_WrongEscSeq;
            }break;
        }
    }
    else{
        result=(lexer->at[0]);
        advance_chars(lexer, 1);
    }
    
    // TODO: think how to recover.
    if(lexer->at[0] != '\''){
        while(!is_newline(lexer->at[0]) &&
              (lexer->at[0] != '\'')){
            advance_chars(lexer, 1);
        }
        if(lexer->at[0] == '\''){
            lexer->error = LError_LargeCharLit;
            advance_chars(lexer, 1);
        }
        else{
            lexer->error = LError_MissedQuote;
        }
        result = 0;
    }
    else{
        advance_chars(lexer, 1);
    }
    
    return(result);
}

/* Reads multi-line string recursively.
   Assumes that the first double quote has already been consumed. */
internal void read_string(Lexer *lexer){
    while(lexer->at[0] && (lexer->at[0] != '\"')){
        if(is_newline(lexer->at[0])){
            lexer->error=LError_NLInConst;
            if(w32_newline(lexer->at[0], lexer->at[1])){
                advance_chars(lexer, 1);
            }
        }
        else if(lexer->at[0] == '\\'){
            advance_chars(lexer, 1);
            switch(lexer->at[0]){
                case('0'): //  NOTE: can be oct escape sequence.
                case('n'):
                case('r'):case('f'):case('t'):case('v'):
                case('b'):case('a'):case('\\'):case('?'):case('\''):
                case('\"'):break;
                default:{
                    lexer->error = LError_WrongEscSeq;
                }break;
            }
        }
        advance_chars(lexer, 1);
    }
    if(lexer->state != LS_PP){
        if(lexer->at[0] == '\"'){
            advance_chars(lexer, 1);
        }
        eat_white(lexer);
        if(lexer->at[0] == '\"'){
            advance_chars(lexer, 1);
            read_string(lexer);
        }
        else if(lexer->at[0] != ';'){
            lexer->error = LError_MissedSemi;
        }
    }
    else{
        if(lexer->at[0] == '\"'){
            advance_chars(lexer, 1);
        }
    }
}

// NOTE: helper function for read_string().
internal void eat_white(Lexer *lexer){
    for(;;){
        while(is_spacing(lexer->at[0])){
            advance_chars(lexer, 1);
        }
        if(is_newline(lexer->at[0])){
            if(w32_newline(lexer->at[0], lexer->at[1])){
                advance_chars(lexer, 1);
            }
            advance_chars(lexer, 1);
            lexer->lastline += 1;
            lexer->lastcolumn = 1;
        }
        else if((lexer->at[0] == '/') && (lexer->at[1] == '/')){
            advance_chars(lexer, 2);
            while(!is_newline(lexer->at[0])){
                advance_chars(lexer, 1);
            }
        }
        else if((lexer->at[0] == '/') && (lexer->at[1] == '*')){
            advance_chars(lexer, 2);
            while(lexer->at[0] &&
                  !((lexer->at[0] == '*') && (lexer->at[1] == '/'))){
                char c = (lexer->at[0]);
                advance_chars(lexer, 1);
                if(is_newline(c)){
                    if(w32_newline(c, lexer->at[0])){
                        advance_chars(lexer, 1);
                    }
                    lexer->lastline += 1;
                    lexer->lastcolumn = 1;
                }
            }
            if((lexer->at[0] == '*') &&
               (lexer->at[1] == '/')){
                advance_chars(lexer, 2);
            }
        }
        else break;
    }
}

// TODO: replace case-statements with macro to reduce the amount of code.
// Example: Case1('&', '&', '=', TK_And, TK_AndAnd, TK_AndEq);

#define Case1(c1, c2, token1, c3, token2, def_token)    \
    case(c1):{                                          \
        if(lexer->at[0] == c2){                         \
            token.kind = token1;                        \
            advance_chars(lexer, 1);                    \
        }                                               \
        else if(lexer->at[0] == c3){                    \
            token.kind = token2;                        \
            advance_chars(lexer, 1);                    \
        }                                               \
        else{                                           \
            token.kind = def_token;                     \
        }                                               \
    }break;

#define Case2(c1, c2, token1, def_token)        \
    case(c1):{                                  \
        if(lexer->at[0] == c2){                 \
            token.kind = token1;                \
            advance_chars(lexer, 1);            \
        }                                       \
        else{                                   \
            token.kind = def_token;             \
        }                                       \
    }break;

#define Case3(c1, c2, token1, c3, token2, c4, token3, def_token)    \
    case(c1):{                                                      \
        if(lexer->at[0] == c2){                                     \
            token.kind = token1;                                    \
            advance_chars(lexer, 1);                                \
        }                                                           \
        else if(lexer->at[0] == c3){                                \
            token.kind = token2;                                    \
            advance_chars(lexer, 1);                                \
        }                                                           \
        else if(lexer->at[0] == c4){                                \
            token.kind = token3;                                    \
            advance_chars(lexer, 1);                                \
        }                                                           \
        else{                                                       \
            token.kind = def_token;                                 \
        }                                                           \
    }break;

#define Case4(c1, c2, c3, token1, c4, token2, c5, token3, def_token)    \
    case(c1):{                                                          \
        if((lexer->at[0] == c2) &&                                      \
           (lexer->at[1] == c3)){                                       \
            token.kind = token1;                                        \
            advance_chars(lexer, 2);                                    \
        }                                                               \
        else if(lexer->at[0] == c4){                                    \
            token.kind = token2;                                        \
            advance_chars(lexer, 1);                                    \
        }                                                               \
        else if(lexer->at[0] == c5){                                    \
            token.kind = token3;                                        \
            advance_chars(lexer, 1);                                    \
        }                                                               \
        else{                                                           \
            token.kind = def_token;                                     \
        }                                                               \
    }break;

#define Case5(c1, c2, c3, token1, def_token)    \
    case(c1):{                                  \
        if((lexer->at[0] == c2) &&              \
           (lexer->at[1] == c3)){               \
            token.kind = token1;                \
            advance_chars(lexer, 2);            \
        }                                       \
        else{                                   \
            token.kind = def_token;             \
        }                                       \
}break;

internal Token gettoken(Lexer *lexer){
    Token token = {0};
    for(;;){
        token.linenumber = lexer->lastline;
        token.columnnumber = lexer->lastcolumn;
        token.lexeme.data = lexer->contents.data;
        lexer->error = LError_Success;
        
        if(is_digit(lexer->at[0])){
            char *p = (lexer->contents.data);
            while(is_digit(p[0])){
                p += 1;
            }
            if((p[0] == '.') ||
               oneof(p[0], "eE")){
                token.kind = TK_FloatNum;
                token.u.F64 = read_float(lexer);
                if(is_alpha(lexer->at[0])){
                    lexer->state = LS_FloatSuf;
                }
            }
            else{
                token.kind = TK_IntNum;
                token.u.U64 = read_integer(lexer, &token);
                if(is_alpha(lexer->at[0])){
                    lexer->state = LS_IntSuf;
                }
            }
        }
        else if((lexer->at[0] == '.') && is_digit(lexer->at[1])){
            token.kind = TK_FloatNum;
            token.u.F64 = read_float(lexer);
            if(is_alpha(lexer->at[0])){
                lexer->state = LS_FloatSuf;
            }
        }
        else{
            char c = (lexer->at[0]);
            advance_chars(lexer, 1);
            
            switch(c){
                case('\0'):{ token.kind = TK_Eoi; }break;
                case('('):{ token.kind = TK_OpenParen; }break;
                case(')'):{ token.kind = TK_CloseParen; }break;
                case('{'):{ token.kind = TK_OpenBrace; }break;
                case('}'):{ token.kind = TK_CloseBrace; }break;
                case('['):{ token.kind = TK_OpenBracket; }break;
                case(']'):{ token.kind = TK_CloseBracket; }break;
                case(','):{ token.kind = TK_Comma; }break;
                case(':'):{ token.kind = TK_Colon; }break;
                case(';'):{ token.kind = TK_Semi; }break;
                case('~'):{ token.kind = TK_Tilde; }break;
                case('@'):{ token.kind = TK_At; }break;
                case('$'):{ token.kind = TK_Dollar; }break;
                case('\\'):{ // multiline macro.
                    if(lexer->state == LS_PP){
                        token.kind = TK_PPslash;
                        lexer->flag |= (1 << 0);
                    }
                    else{
                        token.kind = c;
                    }
                }break;
                case('#'):{
                    if(lexer->state == LS_Default){
                        token.kind = TK_Hash;
                        lexer->state = LS_PP;
                    }
                    else if(lexer->state == LS_PP){
                        if(lexer->at[0] == '#'){
                            token.kind = TK_Concat;
                            advance_chars(lexer, 1);
                        }
                        else{
                            token.kind = TK_Stringize;
                        }
                    }
                }break;
                Case1('&', '&', TK_AndAnd, '=', TK_AndEq, TK_And);
                Case1('|', '|', TK_OrOr, '=', TK_OrEq, TK_Or);
                Case2('^', '=', TK_XorEq, TK_Xor);
                Case1('+', '+', TK_Inc, '=', TK_PlusEq, TK_Plus);
                Case3('-', '-', TK_Dec, '>', TK_Arrow, '=', TK_MinusEq, TK_Minus);
                Case2('*', '=', TK_MulEq, TK_Mul); // NOTE: Maybe TK_Deref as well.
                Case2('%', '=', TK_ModEq, TK_Mod);
                Case2('=', '=', TK_Eq, TK_Asgn);
                Case2('!', '=', TK_NotEq, TK_Not);
                Case4('<', '<', '=', TK_ShiftLEq, '<', TK_ShiftL, '=', TK_LessEq, TK_Less);
                Case4('>', '>', '=', TK_ShiftREq, '>', TK_ShiftR, '=', TK_GrtEq, TK_Grt);
                Case5('.', '.', '.', TK_Dots, TK_Access); // TODO: Rename TK_Dots to TK_Elipsis.
                case('\"'):{
                    token.kind = TK_String;
                    read_string(lexer);
                }break;
                case('\''):{
                    token.kind = TK_IntNum;
                    token.skind = TSK_Char;
                    token.u.U64 = read_char(lexer);
                }break;
                default:{
                    if(is_spacing(c)){
                        while(is_spacing(lexer->at[0])) advance_chars(lexer, 1);
                    }
                    else if((c == '/') && (lexer->at[0] == '/')){
                        advance_chars(lexer, 1);
                        while(!is_newline(lexer->at[0])) advance_chars(lexer, 1);
                    }
                    else if((c == '/') && (lexer->at[0] == '*')){
                        advance_chars(lexer, 1);
                        while(lexer->at[0] &&
                              !((lexer->at[0] == '*') && (lexer->at[1] == '/'))){
                            if(w32_newline(lexer->at[0], lexer->at[1]))
                                advance_chars(lexer, 1);
                            if(is_newline(lexer->at[0]))
                                lexer->lastline += 1;
                            advance_chars(lexer, 1);
                        }
                        if(lexer->at[0] == '*') advance_chars(lexer, 2);
                    }
                    else if(c == '/'){
                        if(lexer->at[0] == '='){
                            token.kind = TK_DivEq;
                            advance_chars(lexer, 1);
                        }
                        else{
                            token.kind = TK_Div;
                        }
                    }
                    else if(is_newline(c)){
                        if(w32_newline(c, lexer->at[0]))
                            advance_chars(lexer, 1);
                        lexer->lastline += 1;
                        lexer->lastcolumn = 1;
                        
                        if(lexer->state == LS_PP){
                            if(!(lexer->flag & (1 << 0))){
                                lexer->state = LS_Default;
                                lexer->flag = 0;
                            }
                            else lexer->flag ^= (1 << 0);
                        }
                    }
                    else if(is_alpha(c) || (c == '_')){
                        switch(lexer->state){
                            case(LS_IntSuf):{
                                lexer->state = LS_Default;
                                if(oneof(c, "uU")){
                                    if(oneof(lexer->at[0], "lL")){
                                        if(oneof(lexer->at[1], "lL")){ token.kind = TK_IntSufULL; advance_chars(lexer, 2); }
                                        else{ token.kind = TK_IntSufUL; advance_chars(lexer, 1); }
                                    }
                                    else token.kind = TK_IntSufU;
                                }
                                else if(oneof(c, "lL")){
                                    if(oneof(lexer->at[0], "lL")){
                                        if(oneof(lexer->at[1], "uU")){ token.kind = TK_IntSufLLU; advance_chars(lexer, 2); } 
                                        else{ token.kind = TK_IntSufLL; advance_chars(lexer, 1); }
                                    }
                                    else if(oneof(lexer->at[0], "uU")){ token.kind = TK_IntSufLU; advance_chars(lexer, 1); } 
                                    else token.kind = TK_IntSufL;
                                }
                                if(is_alpha(lexer->at[0])){
                                    lexer->error = LError_WrongIntSuf;
                                    token.kind = TK_Name; /* Let it be identifier for now. */
                                    while(is_alpha(lexer->at[0])){
                                        advance_chars(lexer, 1);
                                    }
                                }
                            }break;
                            case(LS_FloatSuf):{
                                lexer->state = LS_Default;
                                if(oneof(c, "lL")) token.kind = TK_FloatSufL;
                                else if(oneof(c, "fF")) token.kind = TK_FloatSufF;
                                if(is_alpha(lexer->at[0])){
                                    lexer->error = LError_WrongFloatSuf;
                                    token.kind = TK_Name; /* Let it be identifier for now. */
                                    while(is_alpha(lexer->at[0])){
                                        advance_chars(lexer, 1);
                                    }
                                }
                            }break;
                            case(LS_PP):{
                                /* Make any decisions based on the lexer->flag. */
                                while(is_alnum(lexer->at[0])||
                                      (lexer->at[0] == '_')){
                                    advance_chars(lexer, 1);
                                }
                                token.u.intern = string_intern_range(&intern_table,
                                                                     token.lexeme.data,
                                                                     lexer->contents.data);
                                if(is_ppdir(&token)){
                                    if(lexer->flag & (1 << 1)){
                                        if(token.kind == TK_If_dir){
                                            token.kind = TK_If;
                                        }
                                        else if(token.kind == TK_Else_dir){
                                            token.kind = TK_Else;
                                        }
                                    }
                                    if(!(lexer->flag & (1 << 1))){
                                        lexer->flag |= (1 << 1);
                                    }
                                }
                                else{
                                    if(!is_keyword(&token)){
                                        token.kind = TK_Name;
                                    }
                                }
                            }break;
                            case(LS_Default):{
                                while(is_alnum(lexer->at[0]) ||
                                      (lexer->at[0] == '_')){
                                    advance_chars(lexer, 1);
                                }
                                token.u.intern = string_intern_range(&intern_table,
                                                                     token.lexeme.data,  /* start */
                                                                     lexer->contents.data);
                                if(!is_keyword(&token)){
                                    token.kind = TK_Name;
                                }
                            }break;
                        }
                    }
                    else{   /* Unknown token in C. */
                        token.kind = c;
                    }
                }break;
            }
        }
        
        if(token.kind != TK_Eos){
            token.lexeme.count = (lexer->contents.data - token.lexeme.data);
            break;
        }
    }
    return(token);
}
#undef Case1
#undef Case2
#undef Case3
#undef Case4
#undef Case5

internal void nexttoken(Lexer *lexer){
    if(!is_token(lexer->lookahead, TK_Eos)){
        lexer->token = lexer->lookahead;
        lexer->lookahead.kind = TK_Eos;
    }
    else{
        lexer->token = gettoken(lexer);
    }
}

internal void look_ahead(Lexer *lexer){ 
    if(is_token(lexer->lookahead, TK_Eos)){
        lexer->lookahead = gettoken(lexer);
    }
}

/* Skip lookahead and get a new token. */
internal void next_nexttoken(Lexer *lexer){
    nexttoken(lexer);
    lexer->token = gettoken(lexer);
}

internal b32 is_token(Token token, s32 kind){
    b32 result = (token.kind == kind);
    return(result);
}

internal Lexer init_lexer(String source, String contents){
    Lexer result = {0};
    result.source = source;
    result.contents = contents;
    result.state = LS_Default;
    result.lastline = 1;
    result.lastcolumn = 1;
    result.error = LError_Success;
    refill(&result);
    return(result);
}
