

/*
 * Aleksey Yevtushenko
 * February 13, 2022
 */

#if !defined(LEXER_H)
#define LEXER_H

// NOTE: Not intendet to be complete, just for experiment.
enum{
    LError_Success,
    LError_WrongIntSuf, // incorrect integer suffix.
    LError_WrongFloatSuf, // incorrect float suffix.
    LError_WrongHexSym, // incorrect hex symbol.
    LError_WrongOctSym,
    LError_WrongBinSym,

    LError_U64Overflow,
    LError_F64Overflow,

    LError_EmptyCharLit, // '' and example of empty char literal.
    LError_NLInConst, // new line symbol in either char or string literal.
    LError_WrongEscSeq, // NOT one of {'\n', '\r', '\f', '\t', '\v', '\b', '\a', '\\', '\'', '\"', '\?', '\000', '\xhh'}
    LError_MissedQuote, // missed closed quote in char literal.
    LError_MissedDQuote, // missed closed double quote in string literal.
    LError_LargeCharLit, // 'aaaaa' too many characters in char literal.

    LError_ExpDigit, // missed digit after exponent sign "eE".
    LError_MissedSemi, // missed semicolon (uncomplete statement).
    
    LError_PP_directive_inside_PP,
};

typedef enum TokenKind{
    TK_Eos = 0, // for lookahead.
    
    TK_OpenParen = '(',
    TK_CloseParen = ')',
    TK_OpenBrace = '{',
    TK_CloseBrace = '}',
    TK_OpenBracket = '[',
    TK_CloseBracket = ']',
    TK_Hash = '#', // ambiguous token kind.
    TK_Comma = ',',
    TK_Colon = ':',
    TK_Semi = ';',
    TK_Access = '.', // member selection via dot. e.g.: v3.z
    TK_Asgn = '=',
    TK_Less = '<', // ambiguous token kind.
    TK_Grt = '>', // ambiguous token kind.
    TK_Or = '|',
    TK_And = '&',
    TK_Xor = '^',
    TK_Not = '!',
    TK_Tilde = '~',
    TK_Plus = '+',
    TK_Minus = '-',
    TK_Mul = '*',  // TODO: Should be renamed to TK_Star.
    TK_Div = '/',
    TK_Mod = '%',
    TK_PPslash = '\\',
    
    // NOTE: Invalid tokens in C, but maybe useful for other languages.
    TK_At = '@',
    TK_Dollar = '$',
    
    // Reserved words in C.
    TK_Auto = 256, TK_Double, TK_Int,      TK_Struct,
    TK_Break,      TK_Else,   TK_Long,     TK_Switch,
    TK_Case,       TK_Enum,   TK_Register, TK_Typedef,
    TK_Char,       TK_Extern, TK_Return,   TK_Union,
    TK_Const,      TK_Float,  TK_Short,    TK_Unsigned,
    TK_Continue,   TK_For,    TK_Signed,   TK_Void,
    TK_Default,    TK_Goto,   TK_Sizeof,   TK_Volatile,
    TK_Do,         TK_If,     TK_Static,   TK_While,

    // Preprocessor directives in C.
    // TODO: rename. TK_ifDir -> TK_If_dir;
    TK_If_dir,
    TK_Ifdef_dir,
    TK_Ifndef_dir,
    TK_Elif_dir,
    TK_Else_dir,
    TK_Endif_dir,
    TK_Include_dir,
    TK_Define_dir,
    TK_Undef_dir,
    TK_Line_dir,
    TK_Error_dir,
    TK_Pragma_dir,
    
    TK_defined, // Not preprocessor directive, but has a special role.
    
    // Ambiguous token kinds. Maybe the parser should determine the nature of a token, not lexer.
    TK_Deref,
    TK_Address,
    
    // integer allowed C suffixes.
    TK_IntSufU,
    TK_IntSufL,
    TK_IntSufUL,
    TK_IntSufLU,
    TK_IntSufLLU,
    TK_IntSufULL,
    TK_IntSufLL,

    // float allowed C suffixes.
    TK_FloatSufL,
    TK_FloatSufF,
    
    TK_Dots,
    TK_AndAnd,
    TK_OrOr,
    TK_Inc,
    TK_Dec,
    TK_Eq,
    TK_NotEq,
    TK_ShiftL,
    TK_ShiftR,
    TK_Arrow, 
    TK_AndEq,
    TK_OrEq,
    TK_XorEq,
    TK_PlusEq,
    TK_MinusEq,
    TK_MulEq,
    TK_DivEq,
    TK_ModEq,
    TK_LessEq,
    TK_GrtEq,
    TK_ShiftLEq,
    TK_ShiftREq,

    // Hash has different meaning inside the macro: #-TK_Stringize, ##-TK_Concat.
    TK_Stringize,
    TK_Concat,

    TK_Name,
    TK_IntNum,
    TK_FloatNum,

    TK_Spacing,
    TK_NewLine,

    TK_String,
    TK_Comment,

    TK_Eoi,
}TokenKind;

/* For example TK_IntNum can represent character constants, hex constants, bin and oct constants,
   so we should have a way to distinguish between them. This is the purpose of AuxiliaryKind,
   but you can treat it just as a flag. */
typedef enum TokenSubKind{
    TSK_Unknown,
    TSK_Bin,
    TSK_Oct,
    TSK_Hex,
    TSK_Char, 
}TokenSubKind;

// Generate transition table in future.
typedef enum LexerState{
    LS_Default,
    LS_IntSuf,
    LS_FloatSuf,
    LS_PP,
}LexerState;

typedef struct Token{
    TokenKind kind;     // base token kind.
    TokenSubKind skind; // token sub kind.
    String lexeme;      // just for print, because we have an intern.
    u32 linenumber;
    u32 columnnumber;
    union{
        u64 U64;
        f64 F64;
        String intern;
    }u;
}Token;

typedef struct Lexer{
    String source;    // name of the file being scanned.
    String contents;  // contents of the file being scanned.
    char at[2];       // parse point, at[0] is the current position in the file, and at[1] is lookahead character.
    u32 lastline;     // current line in the source file where the parse point is.
    u32 lastcolumn;   // current column int he source file where the parse point is.
    Token token;      // current token.
    Token lookahead;  // lookahead token, we can use the same technique as at[2].
    Token *tokenbuf;  // STUDY: Do I really need it? buffer of all tokens corresponding to the current source excluding whitespaces.
    LexerState state; // we need this as a flag to recognize some Ambiguous tokens.
    s32 flag;         // additional information for the state.
    s32 error;        // error code
}Lexer;

internal Lexer init_lexer(String source, String input);

internal void refill(Lexer *lexer);
internal void advance_chars(Lexer *lexer, u32 count);

internal u64 read_integer(Lexer *lexer, Token *token);
internal f64 read_float(Lexer *lexer);
internal u64 read_char(Lexer *lexer);
internal void read_string(Lexer *lexer);

internal void eat_white(Lexer *lexer);

internal Token gettoken(Lexer *lexer);
internal void nexttoken(Lexer *lexer);
internal void look_ahead(Lexer *lexer);
internal void next_nexttoken(Lexer *lexer);

internal b32 is_token(Token token, s32 kind);

#endif // LEXER_H
