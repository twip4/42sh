#ifndef COMMON_H
#define COMMON_H

#include "include/builtins/builtins.h"
#include "include/lexer/lexer.h"

typedef struct ShellState
{
    int last_return_code;
    int run_tty;
    token *tokens;
    export_history *exports;
    char **argv;
    int argc;
    int is_interactive;
    int is_subshell;
} ShellState;

#endif // COMMON_H
