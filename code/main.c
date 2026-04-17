

// Aleksey Yevtushenko
// February 13, 2022

#include "common.h"
#include "io.c"
#include "table.c"
#include "lexer.c"
#include "ast.c"
#include "parser.c"
#include "test.c"

int main(int args_count, char **args_values){
    intern_reserved(&intern_table);

    if(args_count >= 2){
        printf("C99 Interpreter — parsing %d file(s)\n\n", args_count - 1);

        for(int i = 1; i < args_count; ++i){
            printf("Reading '%s'...\n", args_values[i]);

            char *file_contents = read_file_into_memory_and_null_terminate(args_values[i]);
            if(!file_contents){
                fprintf(stderr, "  error: could not open '%s'\n", args_values[i]);
                return(1);
            }

            String source = {0};
            source.data = args_values[i];
            source.count = string_len(args_values[i]);

            String contents = {0};
            contents.data = file_contents;
            contents.count = string_len(file_contents);

            printf("  Lexing...\n");
            Lexer lexer = init_lexer(source, contents);

            printf("  Parsing...\n");
            Parser parser = parser_init(&lexer);
            ASTDecl **decls = parse_translation_unit(&parser);

            if(parser.had_error){
                fprintf(stderr, "  " ANSI_RED "Parse failed with errors." ANSI_RESET "\n\n");
            } else {
                s32 num_decls = (s32)darr_len(decls);
                printf("  " ANSI_GREEN "Parse OK" ANSI_RESET
                       " — %d top-level declaration(s) found.\n\n", num_decls);

                s32 num_funcs = 0, num_vars = 0, num_structs = 0;
                s32 num_unions = 0, num_enums = 0, num_typedefs = 0;
                for(s32 j = 0; j < num_decls; ++j){
                    switch(decls[j]->kind){
                        case DECL_FUNC:    num_funcs++;    break;
                        case DECL_VAR:     num_vars++;     break;
                        case DECL_STRUCT:  num_structs++;  break;
                        case DECL_UNION:   num_unions++;   break;
                        case DECL_ENUM:    num_enums++;    break;
                        case DECL_TYPEDEF: num_typedefs++; break;
                        default: break;
                    }
                }

                printf("  Summary:\n");
                if(num_funcs)    printf("    Functions:  %d\n", num_funcs);
                if(num_vars)     printf("    Variables:  %d\n", num_vars);
                if(num_structs)  printf("    Structs:    %d\n", num_structs);
                if(num_unions)   printf("    Unions:     %d\n", num_unions);
                if(num_enums)    printf("    Enums:      %d\n", num_enums);
                if(num_typedefs) printf("    Typedefs:   %d\n", num_typedefs);

                printf("\n  AST dump:\n");
                printf("  ----------------------------------------\n");
                for(s32 j = 0; j < num_decls; ++j){
                    ast_print_decl(decls[j], 1);
                    printf("\n");
                }
                printf("  ----------------------------------------\n");
            }

            free(file_contents);
        }

        printf("Done.\n");
        return(0);
    }

    /* No arguments — run the test suite. */
    printf("C99 Interpreter — running test suite\n\n");

    printf("--- Lexer tests ---\n");
    TEST(test_keywords);
    TEST(test_operators);
    TEST(test_integers);
    TEST(test_floats);
    TEST(test_characters);
    TEST(test_strings);
    TEST(test_comments);
    TEST(test_suffixes);
    TEST(test_preproc);

    printf("\n--- Parser tests ---\n");
    TEST(test_parse_expr);
    TEST(test_parse_declarations);

    printf("\nDone.\n");
    return(0);
}
