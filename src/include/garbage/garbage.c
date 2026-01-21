#include "garbage.h"

#include <stdlib.h>

void **garbage;
int len_garbage = 0;

void add_garbage(void *ptr)
{
    if (!len_garbage)
    {
        garbage = malloc(sizeof(void *));
        garbage[len_garbage++] = ptr;
    }
    else
    {
        void **tmp = realloc(garbage, sizeof(void *) * (len_garbage + 1));
        if (!tmp)
            return;

        garbage = tmp;
        garbage[len_garbage++] = ptr;
    }
}

void clear_garbage()
{
    if (!garbage)
        return;

    for (int i = 0; i < len_garbage; i++)
        free(garbage[i]);

    free(garbage);
    garbage = NULL;
    len_garbage = 0;
}
