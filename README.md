# C99 Interpreter

A hand-written C99 interpreter: lexer, recursive-descent parser, AST, and tree-walking interpreter — all in portable C99.

## Project Status

| Component | Status |
|-----------|--------|
| Lexer | Complete — tokenizes all C99 tokens, keywords, operators, literals, preprocessor directives |
| Parser | Complete — full C99 recursive-descent parser producing an AST |
| AST | Complete — expression, statement, declaration, and type nodes |
| Interpreter | Complete — tree-walking interpreter with functions, structs, pointers, all control flow |

## Quick Start

```sh
make
./interpreter sample.c
```

Running without arguments executes the full test suite:
```sh
./interpreter
```

Use `--ast` to dump the AST before execution:
```sh
./interpreter --ast sample.c
```

## Architecture

```
Source Code  -->  Lexer  -->  Token Stream  -->  Parser  -->  AST  -->  Interpreter  -->  Output
```

### Source Files

| File | Purpose |
|------|---------|
| `src/main.c` | Entry point; includes all `.c` files (single translation unit), dispatches to interpreter or test suite |
| `src/common.h` | Shared types (`s8`–`u64`, `f32`, `f64`, `b32`), `DynamicArray` macros, includes all headers |
| `src/string_guard.h` | `String`/`Buffer` type and character-class/string utility functions |
| `src/table.c` | String interning via linear search (used by lexer for identifiers/keywords) |
| `src/io.c` | File I/O helpers (read file into memory) |
| `src/lexer.h` | Token and Lexer type definitions, lexer API |
| `src/lexer.c` | Full lexer implementation: keywords, operators, numbers, chars, strings, comments, preprocessor |
| `src/ast.h` | AST node definitions: `TypeSpec`, `Expr`, `Stmt`, `ASTDecl`, and constructor prototypes |
| `src/ast.c` | AST node constructors (heap-allocated) and recursive pretty-printer |
| `src/parser.h` | Parser state, API prototypes, and complete EBNF grammar in comments |
| `src/parser.c` | Recursive-descent parser: expressions (15 precedence levels), statements, declarations |
| `src/eval.h` | Interpreter types: `Val` (runtime value), `Scope`/`Env`, `Interp` state, builtin function API |
| `src/eval.c` | Tree-walking interpreter: expression evaluator, statement executor, function calls, builtins |
| `src/test.c` | All tests: lexer, parser AST, and interpreter |
| `grammar.txt` | Standalone reference of the complete implemented grammar |
| `sample.c` | Example C program that exercises all interpreter features |
| `Makefile` | Cross-platform build with `cc` (C99 standard) |
| `build.bat` | Legacy Windows build (MSVC `cl`) |

## Interpreter Features

### Supported C99 Constructs

- **Data types**: `int`, `char`, `short`, `long`, `float`, `double`, `void`, `unsigned`, `signed`
- **Pointers**: `&x`, `*p`, pointer arithmetic, pointer dereferencing as lvalue
- **Structs**: definition, field access (`.`), pointer field access (`->`)
- **Enums**: with implicit and explicit values
- **Functions**: definitions, forward declarations, recursion, varargs (via builtins)
- **Control flow**: `if`/`else`, `while`, `do-while`, `for`, `switch`/`case`/`default`, `break`, `continue`, `goto`/labels
- **Operators**: all C99 arithmetic, bitwise, logical, comparison, assignment, compound assignment, ternary, comma, increment/decrement (prefix and postfix)
- **Type casts**: `(int)x`, `(float)y`
- **sizeof**: for expressions and types

### Built-in Functions

| Function | Description |
|----------|-------------|
| `printf` | Formatted output (supports `%d`, `%i`, `%u`, `%x`, `%o`, `%f`, `%e`, `%g`, `%c`, `%s`, `%p`, `%%`) |
| `putchar` | Write a single character |
| `puts` | Write a string with newline |
| `malloc` | Allocate memory (returns pointer to Val array) |
| `calloc` | Allocate zeroed memory |
| `free` | Free memory (no-op in interpreter) |
| `exit` | Exit with status code |
| `abs` | Absolute value |
| `strlen` | String length |
| `strcmp` | String comparison |
| `atoi` | String to integer |
| `sprintf` | Formatted string output |

### AST Node Families

- **TypeSpec** — Type representations: named types, pointers, arrays, function types, const/volatile qualifiers, struct/union/enum types
- **Expr** — Expressions: integer/float/string/char literals, identifiers, unary/binary/ternary operators, function calls, array subscript, member access, casts, sizeof
- **Stmt** — Statements: expression, return, if/else, while, do-while, for, switch/case/default, break, continue, goto, label, compound block, declaration-as-statement
- **ASTDecl** — Declarations: variables, functions, structs, unions, enums, typedefs

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

## Tests

The test suite runs automatically when executing the binary without arguments:

```sh
./interpreter
```

Coverage:

- **Lexer tests** (9 tests): keywords, operators, integers, floats, characters, strings, comments, suffixes, preprocessor
- **Parser tests** (2 tests): expression AST correctness, full declaration parsing (variables, functions, structs, unions, enums, typedefs, statements)
- **Interpreter tests** (12 tests): arithmetic, variables/globals, control flow (if/while/for/do-while/break/continue), function calls and recursion, switch/case, pointers, structs, enums, bitwise operations, ternary, goto, increment/decrement

## License

GNU General Public License v3.0 — see [LICENSE](LICENSE).
