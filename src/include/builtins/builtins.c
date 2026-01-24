#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../common.h"

int builtin_true(void);
int builtin_false(void);
int builtin_echo(char **argv);
int builtin_exit(void);
int builtin_cd(char **argv);
int builtin_export(char **argv);

extern ShellState g_shell;

int executeur_de_builtins(char **argv)
{
    if (!argv || !argv[0])
        return -1;

    if (strcmp(argv[0], "true") == 0)
        return builtin_true();

    if (strcmp(argv[0], "false") == 0)
        return builtin_false();

    if (strcmp(argv[0], "echo") == 0)
        return builtin_echo(argv);

    if (strcmp(argv[0], "exit") == 0)
        return builtin_exit();

    if (strcmp(argv[0], "cd") == 0)
        return builtin_cd(argv);

    if (strcmp(argv[0], "export") == 0)
        return builtin_export(argv);

    return -1; // BUILTIN PAS IMPLEMENTE
}

// TRUE

int builtin_true(void)
{
    return 0;
}

// FALSE

int builtin_false(void)
{
    return 1;
}

// ECHO

void echo_print(const char *s, int interpret)
{
    for (int i = 0; s[i]; i++)
    {
        if (interpret && s[i] == '\\')
        {
            i++;
            if (s[i] == 'n')
                putchar('\n');
            else if (s[i] == 't')
                putchar('\t');
            else if (s[i] == '\\')
                putchar('\\');
            else
            {
                putchar('\\');
                putchar(s[i]);
            }
        }
        else
            putchar(s[i]);
    }
}

int builtin_echo(char **argv)
{
    int i = 1;
    int newline = 1;
    int interpret = 0;

    while (argv[i] && argv[i][0] == '-')
    {
        if (strcmp(argv[i], "-n") == 0)
            newline = 0;
        else if (strcmp(argv[i], "-e") == 0)
            interpret = 1;
        else if (strcmp(argv[i], "-E") == 0)
            interpret = 0;
        else
            break;
        i++;
    }

    for (; argv[i]; i++)
    {
        echo_print(argv[i], interpret);
        if (argv[i + 1])
            putchar(' ');
    }

    if (newline)
        putchar('\n');

    fflush(stdout);
    return 0;
}

// EXIT

int builtin_exit(void)
{
    g_shell.run_tty = 0;
    return 0;
}

// CD

int builtin_cd(char **argv)
{
    char *path;
    if (!argv[1] || strcmp(argv[1], "~") == 0)
        path = getenv("HOME");
    else if (strcmp(argv[1], "-") == 0)
        path = getenv("OLDPWD");
    else
        path = argv[1];

    char *current_pwd = getcwd(NULL, 0);
    if (chdir(path) != 0) // on fait du coup le chdir et on regarde si ca marche
    {
        fprintf(stderr, "cd: %s: No such file or directory\n", path);
        free(current_pwd);
        return 1;
    }

    if (current_pwd)
    {
        setenv("OLDPWD", current_pwd, 1);
        free(current_pwd);
        current_pwd = NULL;
        current_pwd = getcwd(NULL, 0);
        setenv("PWD", current_pwd, 1);
        free(current_pwd);
    }
    return 0;
}

int split_key_value(const char *str, char **key, char **value)
{
    char *split_point = strchr(str, '=');
    if (!split_point || split_point == str)
        return 0;

    *key = strndup(str, split_point - str);
    *value = strdup(split_point + 1);

    return 1;
}

// EXPORT

int builtin_export(char **argv)
{
    if (argv[1])
    {
        char *key = NULL;
        char *value = NULL;

        if (strcmp(argv[1], "-p") == 0)
        {
            if (argv[2])
            {
                if (!split_key_value(argv[2], &key, &value))
                    return 1;
                setenv(key, value, 1);
            }
            else
                return 1;

            printf("export %s=%s\n", key, value);
        }
        else
        {
            if (!split_key_value(argv[2], &key, &value))
                return 1;
            setenv(key, value, 1);
        }

        free(key);
        free(value);
        return 0;
    }
    else
    {
        return 1;
    }
}
