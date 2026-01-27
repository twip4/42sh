#include "expand.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../../common.h"
#include "../garbage/garbage.h"

int expand_variable(char **s, int len);
char *expand_special(char *val);
char *concat_argv(void);

extern ShellState g_shell; // État global du shell

// Remplace un argument argv[i] par la valeur de la variable
void replace_var(char **argv, int i, char *var)
{
    argv[i] = strdup(var);
    add_garbage(argv[i]);
}
/*
- extrait le nom de la variable à partir d'une chaîne commençant par '$'
- retourne le nom et met à jour stop avec la longueur du token à sauter
*/
char *extract_var(const char *s, int *stop)
{
    if (!s || s[0] != '$')
        return NULL;

    if (s[1] == '{') // Cas ${VAR}
    {
        const char *end = strchr(s + 2, '}'); // Cherche la fin }
        if (!end)
            return NULL;
        *stop = (end - s) + 1; // inclure le '}'
        return strndup(s + 2, end - (s + 2));
    }

    // Variables spéciales d'un seul caractère
    if (s[1] == '?' || s[1] == '$' || s[1] == '*' || s[1] == '@' || s[1] == '#')
    {
        *stop = 2; // $ + char
        return strndup(s + 1, 1);
    }

    // Variables positionnelles numériques $1, $2, ...
    if (isdigit((unsigned char)s[1]))
    {
        *stop = 2;
        return strndup(s + 1, 1);
    }

    // Cas classique : $VAR ou $variable_name
    int len = 1;
    while (s[len] && (isalnum((unsigned char)s[len]) || s[len] == '_'))
        len++;

    *stop = len;
    return strndup(s + 1, len - 1);
}

// Fonction principale d'expansion de variables dans argv
int expand_variable(char **argv, int len)
{
    g_shell.argv = argv; // Met à jour l'état global
    g_shell.argc = len;

    for (int i = 0; i < len; i++)
    {
        char *token = argv[i];
        char buffer[4096] =
            ""; // Buffer temporaire pour construire l'argument développé

        while (*token)
        {
            if (*token == '$') // Détecte une variable
            {
                int stop = 0;
                char *name = extract_var(token, &stop); // Récupère le nom
                if (!name)
                {
                    strncat(buffer, token,
                            1); // Pas de variable trouvée, copie le caractère
                    token++;
                    continue;
                }

                char *value = getenv(
                    name); // Vérifie si c'est une variable d'environnement
                if (!value)
                    value = expand_special(
                        name); // Sinon, regarde les variables spéciales

                if (value)
                    strcat(buffer, value); // Concatène la valeur trouvée

                free(name); // Libère le nom
                token += stop; // suite du token
            }
            else
            {
                strncat(buffer, token, 1); // Copie le caractère non-variable
                token++;
            }
        }

        replace_var(argv, i,
                    buffer); // Remplace argv[i] par la version développée
    }
    return 1;
}

// Valeur du dernier code de retour ($?)
char *expand_status(void)
{
    char *str = malloc(32);
    sprintf(str, "%d", g_shell.last_return_code);
    add_garbage(str);
    return str;
}

// PID du processus courant ($$)
char *expand_pid(void)
{
    char *str = malloc(32);
    sprintf(str, "%d", getpid());
    add_garbage(str);
    return str;
}

// Nombre d'arguments ($#)
char *expand_argc(void)
{
    char *str = malloc(16);
    sprintf(str, "%d", g_shell.argc);
    add_garbage(str);
    return str;
}

// Nombre aléatoire ($RANDOM)
char *expand_random(void)
{
    char *str = malloc(16);
    sprintf(str, "%d", rand());
    add_garbage(str);
    return str;
}

// UID de l'utilisateur ($UID)
char *expand_uid(void)
{
    char *str = malloc(32);
    sprintf(str, "%d", getuid());
    add_garbage(str);
    return str;
}

// Variables positionnelles ($0, $1, ...)
char *expand_positional(char c)
{
    int idx = c - '0';
    char *str;
    if (idx >= g_shell.argc)
        str = strdup("");
    else
        str = strdup(g_shell.argv[idx]);

    add_garbage(str);
    return str;
}

// Concatène tous les arguments en une seule chaîne (pour $* et $@)
char *concat_argv(void)
{
    int full_len = 0;
    for (int i = 0; g_shell.argv[i]; i++)
    {
        full_len += strlen(g_shell.argv[i]) + 1; // +1 pour l'espace
    }

    if (full_len == 0)
        return strdup("");

    char *str = malloc(full_len + 1);
    if (!str)
        return NULL;

    str[0] = '\0';
    for (int i = 0; g_shell.argv[i]; i++)
    {
        strcat(str, g_shell.argv[i]);
        if (g_shell.argv[i + 1])
            strcat(str, " ");
    }

    return str;
}

// Fonction qui retourne la valeur d'une variable spéciale selon son nom
char *expand_special(char *val)
{
    if (!val || val[0] == '\0')
        return NULL;

    if (strcmp(val, "?") == 0)
        return expand_status();

    if (strcmp(val, "$") == 0)
        return expand_pid();

    if (strcmp(val, "*") == 0 || strcmp(val, "@") == 0)
    {
        char *str = concat_argv();
        if (str)
            add_garbage(str);
        return str;
    }
    if (strcmp(val, "#") == 0)
        return expand_argc();

    if (strcmp(val, "RANDOM") == 0)
        return expand_random();

    if (strcmp(val, "UID") == 0)
        return expand_uid();

    if (val[0] >= '0' && val[0] <= '9' && val[1] == '\0')
        return expand_positional(val[0]);

    return NULL; // Variable non reconnue
}
