#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "include/ast/ast.h"
#include "include/executor/executor.h"
#include "include/garbage/garbage.h"
#include "include/lexer/lexer.h"
#include "include/parser/parser.h"

ShellState g_shell = { 0, 1, NULL, NULL, NULL, 0, 0, 0 };

FILE *get_input(int argc, char *argv[])
{
    FILE *input = NULL;

    if (argc >= 2 && strcmp(argv[1], "-c") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "42sh: option -c manque argument\n");
            return NULL;
        }
        input = fmemopen(argv[2], strlen(argv[2]), "r");
        if (!input)
            perror("fmemopen");
    }
    else if (argc >= 2)
    {
        input = fopen(argv[1], "r");
        if (!input)
            perror(argv[1]);
    }
    else
        input = stdin;

    return input;
}

int execution_chain(char *line)
{
    if (!line || line[0] == '\n')
        return 1;

    token *tokens = lexer(line);
    if (!tokens)
        return 1;

    g_shell.tokens = tokens;

    ASTNode *ast = parser(tokens);
    if (!ast)
    {
        fprintf(stderr, "syntax error !\n");
        goto error;
    }

    l_executeur_d_ast(ast);

    token_free_all(tokens);
    free_ast(ast);
    clear_garbage();
    return 1;

error:
    token_free_all(tokens);
    clear_garbage();
    return 0;
}

void shell_loop(FILE *input)
{
    char *line = NULL;
    size_t len = 0;
    int tty = isatty(fileno(input));

    g_shell.is_interactive = tty;

    while (g_shell.run_tty)
    {
        if (tty)
        {
            printf("42sh>");
            fflush(stdout);
        }

        if (getline(&line, &len, input) == -1)
            break;

        if (!execution_chain(line))
            continue;
    }

    free(line);
}

int file_exection(FILE *input)
{
    char *line = NULL;
    size_t len = 0;

    if (getline(&line, &len, input) == -1)
        return 0;

    int ret = execution_chain(line);
    free(line);
    return ret;
}

int main(int argc, char *argv[])
{
    FILE *input = get_input(argc, argv);
    if (!input)
        goto error;

    if (input == stdin)
        shell_loop(input);
    else if (!file_exection(input))
        goto error;

    if (input != stdin)
        fclose(input);

    return 1;

error:
    fprintf(stderr, "error\n");
    clear_garbage();
    return 0;
}
