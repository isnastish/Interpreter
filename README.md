# C99 Interpreter

A hand-written lexer and recursive-descent parser for the C99 programming language.

## Project Status

| Component | Status |
|-----------|--------|
| Lexer | Complete — tokenizes all C99 tokens, keywords, operators, literals, preprocessor directives |
| Parser | Complete — full C99 recursive-descent parser producing an AST |
| AST | Complete — expression, statement, declaration, and type nodes |
| Interpreter | Not yet started |

## Architecture

```
Source Code  -->  Lexer  -->  Token Stream  -->  Parser  -->  AST
```

### Source Files

| File | Purpose |
|------|---------|
| `code/main.c` | Entry point; includes all `.c` files (single translation unit) and runs tests |
| `code/common.h` | Shared types (`s8`–`u64`, `f32`, `f64`, `b32`), `DynamicArray` macros, includes all headers |
| `code/string_guard.h` | `String`/`Buffer` type and character-class/string utility functions |
| `code/table.c` | String interning via linear search (used by lexer for identifiers/keywords) |
| `code/io.c` | File I/O helpers (read file into memory) |
| `code/lexer.h` | Token and Lexer type definitions, lexer API |
| `code/lexer.c` | Full lexer implementation: keywords, operators, numbers, chars, strings, comments, preprocessor |
| `code/ast.h` | AST node definitions: `TypeSpec`, `Expr`, `Stmt`, `ASTDecl`, and constructor prototypes |
| `code/ast.c` | AST node constructors (heap-allocated) and recursive pretty-printer |
| `code/parser.h` | Parser state, API prototypes, and complete EBNF grammar in comments |
| `code/parser.c` | Recursive-descent parser: expressions (15 precedence levels), statements, declarations |
| `code/grammar.txt` | Standalone reference of the complete implemented grammar |
| `code/test.c` | All tests: lexer token tests, expression evaluator tests, AST parser tests |
| `code/Makefile` | Cross-platform build with `cc` (C99 standard) |
| `code/build.bat` | Legacy Windows build (MSVC `cl`) |

### AST Node Families

- **TypeSpec** — Type representations: named types (`int`, `char`, user-defined), pointers, arrays, function types, const/volatile qualifiers, struct/union/enum types
- **Expr** — Expressions: integer/float/string/char literals, identifiers, unary/binary/ternary operators, function calls, array subscript, member access (`.` and `->`), casts, sizeof, parenthesised
- **Stmt** — Statements: expression, return, if/else, while, do-while, for, switch/case/default, break, continue, goto, label, compound block, declaration-as-statement
- **ASTDecl** — Declarations: variables (with optional initializer), functions (definition and forward declaration), structs, unions, enums, typedefs

### Expression Precedence (lowest to highest)

1. Comma (`,`)
2. Assignment (`=`, `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=`)
3. Ternary (`?:`)
4. Logical OR (`||`)
5. Logical AND (`&&`)
6. Bitwise OR (`|`)
7. Bitwise XOR (`^`)
8. Bitwise AND (`&`)
9. Equality (`==`, `!=`)
10. Relational (`<`, `>`, `<=`, `>=`)
11. Shift (`<<`, `>>`)
12. Additive (`+`, `-`)
13. Multiplicative (`*`, `/`, `%`)
14. Cast (`(type)`)
15. Unary (`++`, `--`, `&`, `*`, `+`, `-`, `~`, `!`, `sizeof`)
16. Postfix (`()`, `[]`, `.`, `->`, `++`, `--`)

## Building

### macOS / Linux

```sh
cd code
make
./interpreter
```

### Windows (MSVC)

```cmd
code\build.bat
```

## Tests

The test suite runs automatically when executing the built binary. It covers:

- **Lexer tests** (9 tests): keywords, operators, integers, floats, characters, strings, comments, suffixes, preprocessor
- **Legacy expression evaluator** (1 test): constant integer arithmetic
- **Parser tests** (9 sub-tests inside `test_parse_declarations`):
  - Expression AST: precedence, associativity, ternary, calls, subscripts, member access, casts, sizeof, postfix
  - Statements: if/else, while, do-while, for, switch/case/default, return, break, continue, goto, label, blocks
  - Variable declarations: basic, pointers, arrays, const, unsigned long long, static, extern
  - Function declarations: definitions with body, forward declarations, varargs, static linkage
  - Struct declarations: named, anonymous, forward declarations, nested types
  - Union declarations: named, anonymous
  - Enum declarations: simple, with explicit values, trailing comma
  - Typedef declarations: basic, pointer types
  - Translation unit: multi-declaration source parsed end-to-end

## License

GNU General Public License v3.0 — see [LICENSE](LICENSE).
