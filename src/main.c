

// Aleksey Yevtushenko
// February 13, 2022

#include "common.h"
#include "io.c"
#include "table.c"
#include "lexer.c"
#include "ast.c"
#include "parser.c"
#include "eval.c"
#include "test.c"

int main(int args_count, char **args_values){
    intern_reserved(&intern_table);

    if(args_count >= 2){
        b32 dump_ast = 0;

        /* Check for --ast flag */
        int file_start = 1;
        if(args_count >= 3 && strcmp(args_values[1], "--ast") == 0){
            dump_ast = 1;
            file_start = 2;
        }

        for(int i = file_start; i < args_count; ++i){
            char *file_contents = read_file_into_memory_and_null_terminate(args_values[i]);
            if(!file_contents){
                fprintf(stderr, "error: could not open '%s'\n", args_values[i]);
                return(1);
            }

            String source = {0};
            source.data = args_values[i];
            source.count = string_len(args_values[i]);

            String contents = {0};
            contents.data = file_contents;
            contents.count = string_len(file_contents);

            Lexer lexer = init_lexer(source, contents);
            Parser parser = parser_init(&lexer);
            ASTDecl **decls = parse_translation_unit(&parser);

            if(parser.had_error){
                fprintf(stderr, ANSI_RED "Parse failed with errors." ANSI_RESET "\n");
                free(file_contents);
                return(1);
            }

            if(dump_ast){
                s32 num_decls = (s32)darr_len(decls);
                printf("--- AST for '%s' (%d declarations) ---\n\n", args_values[i], num_decls);
                for(s32 j = 0; j < num_decls; ++j){
                    ast_print_decl(decls[j], 0);
                    printf("\n");
                }
            }

            /* Run the interpreter */
            Interp interp = interp_init();
            interp_run(&interp, decls);

            if(interp.had_error){
                fprintf(stderr, ANSI_RED "Runtime error occurred." ANSI_RESET "\n");
                free(file_contents);
                return(1);
            }

            free(file_contents);
        }

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

    printf("\n--- Interpreter tests ---\n");
    TEST(test_interp_arithmetic);
    TEST(test_interp_variables);
    TEST(test_interp_control_flow);
    TEST(test_interp_functions);
    TEST(test_interp_switch);
    TEST(test_interp_pointers);
    TEST(test_interp_structs);
    TEST(test_interp_enums);
    TEST(test_interp_bitwise);
    TEST(test_interp_ternary);
    TEST(test_interp_goto);
    TEST(test_interp_increment);

    printf("\nDone.\n");
    return(0);
}
