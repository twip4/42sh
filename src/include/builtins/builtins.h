#ifndef BUILTINS_H
#define BUILTINS_H

typedef struct export_history
{
    char *key;
    struct export_history *next;
} export_history;

int executeur_de_builtins(char **argv);

#endif // BUILTINS_H
