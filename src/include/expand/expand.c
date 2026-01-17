#include "expand.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../../42sh.h"

int expand_variable(char **s, int len);
int expand_special(char **argv, int i);

void replace_var(char **argv, int i, char *var)
{
    char *tmp = argv[i];
    argv[i] = strdup(var);
    free(tmp);
}

int expand_ast(ASTNode *ast)
{
    if (!ast)
        return 1;

    switch (ast->type)
    {
    case AST_COMMAND:
        return expand_variable(ast->data.command.argv, ast->data.command.len);
        break;
    case AST_REDIRECT:
        return expand_variable(&ast->data.redirect.filename, 1);
        break;
    case AST_PIPE:
    case AST_AND:
    case AST_OR:
        return expand_ast(ast->data.child.left)
            && expand_ast(ast->data.child.right);
        break;
    case AST_SUBSHELL:
        return expand_ast(ast->data.subshell.child);
        break;
    }

    return 1;
}

int expand_variable(char **s, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (s[i][0] == '$')
        {
            char *var = getenv(s[i] + 1);

            if (var)
            {
                replace_var(s, i, var);
            }
            else
            {
                return expand_special(s, i);
            }
        }
    }
    return 1;
}

int expand_special(char **argv, int i)
{
    if (!strcmp(argv[i], "$?")) // valeur de retour
    {
        char str[10] = "";
        sprintf(str, "%d", last_return_code);
        replace_var(argv, i, str);
        return 1;
    }
    else if (!strcmp(argv[i], "$$")) // pid du shell
    {
        pid_t shell_pid = getpid();
        char str[50] = "";
        sprintf(str, "%d", shell_pid);
        replace_var(argv, i, str);
        return 1;
    }
    else if (!strcmp(argv[i], "$*")) // TODO:
    {
        pid_t shell_pid = getpid();
        char str[50] = "";
        sprintf(str, "%d", shell_pid);
        replace_var(argv, i, str);
        return 1;
    }
    else if (!strcmp(argv[i], "$@")) // TODO:
    {
        pid_t shell_pid = getpid();
        char str[50] = "";
        sprintf(str, "%d", shell_pid);
        replace_var(argv, i, str);
        return 1;
    }
    else if (!strcmp(argv[i], "$#")) // TODO:
    {
        pid_t shell_pid = getpid();
        char str[50] = "";
        sprintf(str, "%d", shell_pid);
        replace_var(argv, i, str);
        return 1;
    }
    else if (!strcmp(argv[i], "$RANDOM")) // random int32
    {
        int tmp = rand();
        char str[10] = "";
        sprintf(str, "%d", tmp);
        replace_var(argv, i, str);
        return 1;
    }
    else if (!strcmp(argv[i], "$UID")) // uid user
    {
        uid_t uid = getuid();
        char str[50] = "";
        sprintf(str, "%d", uid);
        replace_var(argv, i, str);
        return 1;
    }
    else if (argv[i][0] == '$'
             && (argv[i][1] >= '0' && argv[i][1] <= '9')) // TODO:
    {
        pid_t shell_pid = getpid();
        char str[50] = "";
        sprintf(str, "%d", shell_pid);
        replace_var(argv, i, str);
        return 1;
    }
    else
    {
        return 0;
    }
}
