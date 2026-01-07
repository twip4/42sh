#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


static void le_liseur_afficheur(FILE *input)
{
    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, input) != -1){
        printf("%s", line);
    }

    free(line);
}

int main(int argc, char *argv[])
{
    FILE *input = NULL;

    if (argc >= 2 && strcmp(argv[1], "-c") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "42sh: option -c manque qrgument\n");
            return 1;
        }

        input = fmemopen(argv[2], strlen(argv[2]), "r");
        if (!input)
        {
            perror("fmemopen");
            return 1;
        }
    }
    else if (argc >= 2)
    {
        input = fopen(argv[1], "r");
        if (!input)
        {
            perror(argv[1]);
            return 1;
        }
    }
    else
    {
        input = stdin;
    }

    le_liseur_afficheur(input);

    if (input != stdin){
        fclose(input);
        //pour fmemopen et fopen. pas besoin pour stdin c'est gere par le systeme
    }

    return 0;
}
