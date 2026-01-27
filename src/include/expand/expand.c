#include "expand.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../../common.h"
#include "../garbage/garbage.h"

int expand_variable(char **s, int len);
char *expand_special(char *val);
char *concat_argv(void);

extern ShellState g_shell;

void replace_var(char **argv, int i, char *var)
{
    argv[i] = strdup(var);
    add_garbage(argv[i]);
}

char *extract_var(const char *s, int *stop)
{
    if (!s || s[0] != '$')
        return NULL;

    if (s[1] == '{')
    {
        const char *end = strchr(s + 2, '}');
        if (!end)
            return NULL;
        *stop = (end - s) + 1; // inclure le '}'
        return strndup(s + 2, end - (s + 2));
    }

    if (s[1] == '?' || s[1] == '$' || s[1] == '*' || s[1] == '@' || s[1] == '#')
    {
        *stop = 2; // $ + char
        return strndup(s + 1, 1);
    }

    if (isdigit((unsigned char)s[1]))
    {
        *stop = 2;
        return strndup(s + 1, 1);
    }

    int len = 1;
    while (s[len] && (isalnum((unsigned char)s[len]) || s[len] == '_'))
        len++;

    *stop = len;
    return strndup(s + 1, len - 1);
}

int expand_variable(char **argv, int len)
{
    g_shell.argv = argv;
    g_shell.argc = len;

    for (int i = 0; i < len; i++)
    {
        char *token = argv[i];
        char buffer[4096] = "";

        while (*token)
        {
            if (*token == '$')
            {
                int stop = 0;
                char *name = extract_var(token, &stop);
                if (!name)
                {
                    strncat(buffer, token, 1);
                    token++;
                    continue;
                }

                char *value = getenv(name);
                if (!value)
                    value = expand_special(name);

                if (value)
                    strcat(buffer, value);

                free(name);
                token += stop;
            }
            else
            {
                strncat(buffer, token, 1);
                token++;
            }
        }

        replace_var(argv, i, buffer);
    }
    return 1;
}

char *expand_status(void)
{
    char *str = malloc(32);
    sprintf(str, "%d", g_shell.last_return_code);
    add_garbage(str);
    return str;
}

char *expand_pid(void)
{
    char *str = malloc(32);
    sprintf(str, "%d", getpid());
    add_garbage(str);
    return str;
}

char *expand_argc(void)
{
    char *str = malloc(16);
    sprintf(str, "%d", g_shell.argc);
    add_garbage(str);
    return str;
}

char *expand_random(void)
{
    char *str = malloc(16);
    sprintf(str, "%d", rand());
    add_garbage(str);
    return str;
}

char *expand_uid(void)
{
    char *str = malloc(32);
    sprintf(str, "%d", getuid());
    add_garbage(str);
    return str;
}

char *expand_positional(char c)
{
    int idx = c - '0';
    char *str;
    if (idx >= g_shell.argc)
        str = strdup("");
    else
        str = strdup(g_shell.argv[idx]);

    add_garbage(str);
    return str;
}

char *concat_argv(void)
{
    int full_len = 0;
    for (int i = 0; g_shell.argv[i]; i++)
    {
        full_len += strlen(g_shell.argv[i]) + 1;
    }

    if (full_len == 0)
        return strdup("");

    char *str = malloc(full_len + 1);
    if (!str)
        return NULL;

    str[0] = '\0';
    for (int i = 0; g_shell.argv[i]; i++)
    {
        strcat(str, g_shell.argv[i]);
        if (g_shell.argv[i + 1])
            strcat(str, " ");
    }

    return str;
}

char *expand_special(char *val)
{
    if (!val || val[0] == '\0')
        return NULL;

    if (strcmp(val, "?") == 0)
        return expand_status();

    if (strcmp(val, "$") == 0)
        return expand_pid();

    if (strcmp(val, "*") == 0 || strcmp(val, "@") == 0)
    {
        char *str = concat_argv();
        if (str)
            add_garbage(str);
        return str;
    }
    if (strcmp(val, "#") == 0)
        return expand_argc();

    if (strcmp(val, "RANDOM") == 0)
        return expand_random();

    if (strcmp(val, "UID") == 0)
        return expand_uid();

    if (val[0] >= '0' && val[0] <= '9' && val[1] == '\0')
        return expand_positional(val[0]);

    return NULL;
}
