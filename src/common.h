#ifndef COMMON_H
#define COMMON_H

#include "include/builtins/builtins.h"

typedef struct ShellState
{
    int last_return_code;
    int run_tty;
    export_history *exports;
} ShellState;

#endif // COMMON_H
