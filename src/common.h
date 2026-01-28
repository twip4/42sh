#ifndef COMMON_H
#define COMMON_H

#include "include/builtins/builtins.h"
#include "include/lexer/lexer.h"

typedef enum loop_control
{
    CTRL_NONE,
    CTRL_BREAK,
    CTRL_CONTINUE
} loop_control;

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
    loop_control ctrl;
    int in_loop;
} ShellState;

#endif // COMMON_H
