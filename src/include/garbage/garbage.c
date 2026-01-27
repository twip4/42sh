#include "garbage.h"

#include <stdlib.h>

void **garbage; // Tableau global de pointeurs pour suivre la "mémoire à libérer"
int len_garbage = 0; // Nombre d'éléments actuellement dans le tableau garbage

/*
Ajoute un pointeur à la liste de "garbage" pour le libérer plus tard.
ptr : pointeur à ajouter
*/
void add_garbage(void *ptr)
{
    if (!len_garbage) // // Si c'est le premier élément ajouté, on alloue la memoire + incrementation de la len
    {
        garbage = malloc(sizeof(void *));
        garbage[len_garbage++] = ptr;
    }
    else // sinon , on redimensionne le tableau pour ajouter un nouvel élément
    {
        void **tmp = realloc(garbage, sizeof(void *) * (len_garbage + 1));
        if (!tmp)
            return;

        garbage = tmp;
        garbage[len_garbage++] = ptr;
    }
}

// Libère toute la mémoire stockée dans le tableau garbage
void clear_garbage(void)
{
    if (!garbage)
        return;
    // Parcours tous les pointeurs stockés et les libère
    for (int i = 0; i < len_garbage; i++)
        free(garbage[i]);

    free(garbage); // Libère le tableau lui-même
    garbage = NULL; // Réinitialise le pointeur global
    len_garbage = 0; // Réinitialise le compteur
}
