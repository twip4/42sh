#include "builtins.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../common.h"

int builtin_true(void);
int builtin_false(void);
int builtin_echo(char **argv);
int builtin_exit(char **argv);
int builtin_cd(char **argv);
int builtin_export(char **argv);
void clear_exports(void);
int builtin_unset(char **argv);
int builtin_dot(char **argv);

// Variable globale pour l'état du shell
extern ShellState g_shell;

// Fonction principale qui exécute un builtin si reconnu
int executeur_de_builtins(char **argv)
{
    if (!argv || !argv[0])
        return -1; // aucun argument : pas de builtin à exécuter

    // Vérification et exécution selon le nom du builtin
    if (strcmp(argv[0], "true") == 0)
        return builtin_true();

    if (strcmp(argv[0], "false") == 0)
        return builtin_false();

    if (strcmp(argv[0], "echo") == 0)
        return builtin_echo(argv);

    if (strcmp(argv[0], "exit") == 0)
        return builtin_exit(argv);

    if (strcmp(argv[0], "cd") == 0)
        return builtin_cd(argv);

    if (strcmp(argv[0], "export") == 0)
        return builtin_export(argv);

    if (strcmp(argv[0], "unset") == 0)
        return builtin_unset(argv);

    if (strcmp(argv[0], ".") == 0)
        return builtin_dot(argv);

    return -1; // BUILTIN PAS IMPLEMENTE
}

// TRUE

int builtin_true(void)
{
    return 0; // code retour shell : succès
}

// FALSE

int builtin_false(void)
{
    return 1; // code retour shell : échec
}

// ECHO

void echo_print(const char *s, int interpret)
{
    // Affiche la chaîne s, interprète éventuellement les caractères échappés
    for (int i = 0; s[i]; i++)
    {
        if (interpret && s[i] == '\\')
        {
            i++;
            if (s[i] == 'n') // interprétation de \n
                putchar('\n');
            else if (s[i] == 't') // interprétation de \t
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
            putchar(s[i]); // affichage normal
    }
}

int builtin_echo(char **argv)
{
    int i = 1; // début des arguments après "echo"
    int newline = 1; // par défaut, on ajoute un saut de ligne
    int interpret = 0; // par défaut, pas d'interprétation des \n, \t

    while (argv[i] && argv[i][0] == '-')
    {
        if (strcmp(argv[i], "-n") == 0)
            newline = 0; // pas de saut de ligne
        else if (strcmp(argv[i], "-e") == 0)
            interpret = 1; // interpréter les échappements
        else if (strcmp(argv[i], "-E") == 0)
            interpret = 0; // ne pas interpréter
        else
            break; // autre option inconnue : fin des options
        i++;
    }

    // Affichage des arguments
    for (; argv[i]; i++)
    {
        echo_print(argv[i], interpret);
        if (argv[i + 1])
            putchar(' '); // espace entre les mots
    }

    if (newline)
        putchar('\n');

    fflush(stdout); // forcer l'affichage
    return 0;
}

// EXIT

int is_number(char *s)
{
    int i = 0;
    while (s[i] != '\0')
    {
        if (!isdigit(s[i]))
            return 0;
        i++;
    }
    return 1;
}

int builtin_exit(char **argv)
{
    long return_code;

    if (argv[1] && argv[2])
    {
        fprintf(stderr, "too many arguments\n");
        return 1;
    }

    if (argv[1])
    {
        if (!is_number(argv[1]))
        {
            fprintf(stderr, "exit %s is not numeric arguments\n", argv[1]);
            return_code = 2;
        }
        else
            return_code = atoi(argv[1]);
    }
    else
        return_code = g_shell.last_return_code;

    if (g_shell.is_interactive && !g_shell.is_subshell)
        printf("exit\n");

    clear_exports();
    // clean_shell(); TODO:

    if (g_shell.is_subshell)
        _exit(return_code);
    else
        exit(return_code);
}

// CD

int builtin_cd(char **argv)
{
    char *path;
    // choix du chemin à utiliser
    if (!argv[1] || strcmp(argv[1], "~") == 0)
        path = getenv("HOME"); // par défaut : HOME
    else if (strcmp(argv[1], "-") == 0)
        path = getenv("OLDPWD"); // dernier dossier
    else
        path = argv[1]; // chemin fourni

    char *current_pwd = getcwd(NULL, 0); // sauvegarde du pwd actuel
    if (chdir(path) != 0) // on fait du coup le chdir et on regarde si ca marche
    {
        fprintf(stderr, "cd: %s: No such file or directory\n", path);
        free(current_pwd);
        return 1;
    }

    // Mise à jour des variables d'environnement PWD et OLDPWD
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

// Sépare une chaîne de la forme "KEY=VALUE"
int split_key_value(const char *str, char **key, char **value)
{
    char *split_point = strchr(str, '=');
    if (!split_point || split_point == str)
        return 0; // pas de = trouvé ou clé vide

    *key = strndup(str, split_point - str);
    *value = strdup(split_point + 1);

    return 1;
}

// EXPORT

// Ajoute une clé dans l'historique des exports
void add_export(char *key)
{
    export_history *node = malloc(sizeof(export_history));

    node->key = key;
    node->next = NULL;

    if (!g_shell.exports)
    {
        g_shell.exports = node;
        return;
    }
    // ajout à la fin de la liste
    export_history *current = g_shell.exports;
    while (current->next)
        current = current->next;
    current->next = node;
}

// Affiche tous les exports
void print_exports(void)
{
    export_history *current = g_shell.exports;
    while (current)
    {
        char *value = getenv(current->key);
        if (value)
            printf("export %s=%s\n", current->key, value);
        else
            printf("export %s\n", current->key);
        current = current->next;
    }
}

// Libère la mémoire des exports
void clear_exports(void)
{
    export_history *curr = g_shell.exports;
    while (curr)
    {
        export_history *next = curr->next;
        free(curr->key);
        free(curr);
        curr = next;
    }

    g_shell.exports = NULL;
}

int builtin_export(char **argv)
{
    if (argv[1])
    {
        char *key = NULL;
        char *value = NULL;

        if (strcmp(argv[1], "-p") == 0)
        {
            print_exports();
        }
        else
        {
            if (!split_key_value(argv[1], &key, &value))
                return 1; // format invalide
            add_export(key); // ajouter à l'historique
            setenv(key, value, 1); // exporter dans l'environnement
        }

        free(value); // libération mémoire
        return 0;
    }
    else
    {
        return 1;
    }
}

// UNSET

int builtin_unset(char **argv) // rajouter les fonctions
{
    if (argv[1])
    {
        int i = 1;
        while (argv[i])
        {
            unsetenv(argv[i]); // supprime la variable d'environnement
            i++;
        }
        return 0;
    }
    fprintf(stderr, "invalid args\n");
    return 1; // erreur si aucun argument
}

int builtin_dot(char **argv)
{
    FILE *file;
    char *line = NULL;
    size_t len = 0;
    int last_status = 0;

    if (!argv[1])
    {
        fprintf(stderr, ".: filename argument required\n");
        return 2;
    }
    file = fopen(argv[1], "r");
    if (!file)
    {
        fprintf(stderr, ".: %s: No such file or directory\n", argv[1]);
        return 1;
    }

    while (getline(&line, &len, file) != -1)
    {
        if (line[0] == '\n' || line[0] == '#')
            continue;
        last_status = execution_chain(line);
        g_shell.last_return_code = last_status;
    }
    free(line);
    fclose(file);
    return last_status;
}
