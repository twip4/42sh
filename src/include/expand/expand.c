#include "expand.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../../common.h"
#include "../garbage/garbage.h"

int expand_variable(char **s, int len);
int expand_special(char **argv, int i);
char *concat_argv(char **argv, int len);

extern ShellState g_shell;

void replace_var(char **argv, int i, char *var)
{
    argv[i] = strdup(var);
    add_garbage(argv[i]);
}

int expand_if_branches(IfBranch *branch)
{
    while (branch)
    {
        if (!expand_ast(branch->condition))
            return 0;
        if (!expand_ast(branch->bloc))
            return 0;

        branch = branch->next;
    }
    return 1;
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
    case AST_CHAIN:
        return expand_ast(ast->data.child.left)
            && expand_ast(ast->data.child.right);
        break;
    case AST_SUBSHELL:
        return expand_ast(ast->data.subshell.child);
        break;
    case AST_IF:
        if (!expand_if_branches(ast->data.ifnode.branches))
            return 0;
        return expand_ast(ast->data.ifnode.blocelse);

    default:
        return 0;
    }

    return 1;
}

char *extract_var(char *s) // TODO: better implem
{
    if (s[0] == '$')
    {
        if (s[1] == '{')
        {
            char *var_end = strchr(s + 2, '}');
            if (!var_end)
                return NULL;

            return strndup(s + 2, var_end - (s + 2));
        }
        else
        {
            int len = 1;

            while (s[len] != '\0' && s[len] >= 'A' && s[len] <= 'Z')
                len++;

            return strndup(s + 1, len);
        }
    }
    else
        return NULL;
}

int expand_variable(char **s, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (s[i][0] == '$')
        {
            char *name = extract_var(s[i]);

            if (!name)
                continue;

            char *var = getenv(name);

            if (var)
            {
                replace_var(s, i, var);
            }
            else
            {
                expand_special(s, i);
            }

            free(name);
        }
    }
    return 1;
}

int expand_status(char **argv, int i)
{
    if (strcmp(argv[i], "$?"))
        return 0;

    char str[16];
    sprintf(str, "%d", g_shell.last_return_code);
    replace_var(argv, i, str);
    return 1;
}

int expand_pid(char **argv, int i)
{
    if (strcmp(argv[i], "$$"))
        return 0;

    char str[32];
    sprintf(str, "%d", getpid());
    replace_var(argv, i, str);
    return 1;
}

int expand_argv_all(char **argv, int i)
{
    if (strcmp(argv[i], "$*") && strcmp(argv[i], "$@"))
        return 0;

    char *str = concat_argv(argv, i);
    if (str)
    {
        replace_var(argv, i, str);
        free(str);
    }
    return 1;
}

int expand_argc(char **argv, int i)
{
    if (strcmp(argv[i], "$#"))
        return 0;

    char str[16];
    sprintf(str, "%d", i);
    replace_var(argv, i, str);
    return 1;
}

int expand_random(char **argv, int i)
{
    if (strcmp(argv[i], "$RANDOM"))
        return 0;

    char str[16];
    sprintf(str, "%d", rand());
    replace_var(argv, i, str);
    return 1;
}

int expand_uid(char **argv, int i)
{
    if (strcmp(argv[i], "$UID"))
        return 0;

    char str[32];
    sprintf(str, "%d", getuid());
    replace_var(argv, i, str);
    return 1;
}

int expand_positional(char **argv, int i)
{
    if (argv[i][0] != '$' || argv[i][1] < '0' || argv[i][1] > '9')
        return 0;

    int idx = argv[i][1] - '0';
    if (!argv[idx])
        replace_var(argv, i, "");
    else
        replace_var(argv, i, argv[idx]);

    return 1;
}

int expand_special(char **argv, int i)
{
    if (expand_status(argv, i))
        return 1;
    if (expand_pid(argv, i))
        return 1;
    if (expand_argv_all(argv, i))
        return 1;
    if (expand_argc(argv, i))
        return 1;
    if (expand_random(argv, i))
        return 1;
    if (expand_uid(argv, i))
        return 1;
    if (expand_positional(argv, i))
        return 1;

    return 0;
}

char *concat_argv(char **argv, int len)
{
    int full_len = 0;

    if (len < 2)
        return NULL;

    for (int i = 2; i < len; i++)
    {
        full_len += strlen(argv[i]);
        full_len++;
    }

    char *str_concat = malloc(full_len);

    if (!str_concat)
        return NULL;

    for (int i = 2; i < len; i++)
    {
        strcat(str_concat, argv[i]);
    }

    return str_concat;
}
