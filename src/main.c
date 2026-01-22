#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "include/ast/ast.h"
#include "include/executor/executor.h"
#include "include/expand/expand.h"
#include "include/garbage/garbage.h"
#include "include/lexer/lexer.h"
#include "include/parser/parser.h"

int last_return_code = 0;
int run_tty = 1;

int main(int argc, char *argv[])
{
    FILE *input = NULL;
    int tty;

    if (argc >= 2 && strcmp(argv[1], "-c") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "42sh: option -c manque argument\n");
            return 1;
        }

        input = fmemopen(argv[2], strlen(argv[2]), "r");
        if (!input)
        {
            perror("fmemopen");
            return 1;
        }
    }
    else if (argc >= 2)
    {
        input = fopen(argv[1], "r");
        if (!input)
        {
            perror(argv[1]);
            return 1;
        }
    }
    else
    {
        input = stdin;
    }

    tty = isatty(fileno(input));

    char *line = NULL;
    size_t len = 0;

    if (input == stdin)
    {
        while (run_tty)
        {
            if (tty)
            {
                printf("42sh>");
                fflush(stdout);
            }
            ssize_t r = getline(&line, &len, input);
            if (r == -1)
                break;

            token *tokens = lexer(line);
            if (!tokens)
                goto error;

            ASTNode *ast = parser(tokens);
            if (!ast)
                goto error;

            if (!expand_ast(ast))
                goto error;

            // ast_pretty_print(ast, 0);

            last_return_code = l_executeur_d_ast(ast);

            token_free_all(tokens);
            free_ast(ast);
            clear_garbage();
        }

        free(line);
        line = NULL;
    }
    else
    {
        ssize_t r = getline(&line, &len, input);
        if (r == -1)
            goto error;

        token *tokens = lexer(line);
        if (!tokens)
            goto error;

        ASTNode *ast = parser(tokens);
        if (!ast)
            goto error;

        ast_pretty_print(ast, 0);

        last_return_code = l_executeur_d_ast(ast);

        token_free_all(tokens);
        free_ast(ast);
        clear_garbage();
    }

    if (input != stdin)
    {
        fclose(input);
        // pour fmemopen et fopen. pas besoin pour stdin c'est gere par le
        // systeme
    }

    return 0;

error:
    fprintf(stderr, "error \n");
    clear_garbage();
    return 1;
}
