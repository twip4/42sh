#include "builtins.h"

#include <stdio.h>
#include <string.h>

#include "../../common.h"

static int builtin_true(void);
static int builtin_false(void);
static int builtin_echo(char **argv);
int builtin_exit(void);

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

    return -1; // BUILTIN PAS IMPLEMENTE
}

// TRUE

static int builtin_true(void)
{
    return 0;
}

// FALSE

static int builtin_false(void)
{
    return 1;
}

// ECHO

static void echo_print(const char *s, int interpret)
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

static int builtin_echo(char **argv)
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

int builtin_exit(void)
{
    run_tty = 0;
    return 0;
}
