#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

token *token_init(void)
{
    token *head;

    head = malloc(sizeof(token));
    if (!head)
        return NULL;
    head->type = -1; // token initial, ne représente rien
    head->value = NULL;
    head->next = NULL;
    return head;
}

// crée un token avec un type et une valeur. La valeur peut être NULL pour des
// symboles (|, >, etc.).
token *token_new(token_type type, char *value)
{
    token *t = malloc(sizeof(token));
    if (!t)
        return NULL;
    t->type = type;
    t->value = value;
    t->next = NULL;
    return t;
}

// ajoute un token à la fin de la liste chaînée.
void token_add(token *head, token *new)
{
    token *tmp = head;

    while (tmp->next)
        tmp = tmp->next;
    tmp->next = new;
}

// libère toute la mémoire des tokens (valeurs + structure)
void token_free_all(token *head)
{
    token *tmp;
    token *current = head;

    while (current)
    {
        tmp = current->next;
        if (current->value)
        {
            free(current->value); // libération de la valeur
        }
        free(current); // libération du token
        current = tmp;
    }
}

// Crée et ajoute un token en une seule opération
int add_token(token *head, token_type type, char *value)
{
    token *new = token_new(type, value);
    if (!new)
        return 0;
    token_add(head, new);
    return 1;
}

// Gestion des parties de mots
char *append_part(char *res, char *part)
{
    // Concatène 'part' à 'res', gère la mémoire avec realloc
    int len_res = res ? strlen(res) : 0;
    int len_part = strlen(part);
    char *tmp = realloc(res,
                        len_res + len_part
                            + 1); // échec realloc : libère res et retourne NULL

    if (!tmp)
        return free(res), NULL;

    memcpy(tmp + len_res, part, len_part); // copie la nouvelle partie
    tmp[len_res + len_part] = '\0'; // terminaison de chaîne
    return tmp;
}

// Lit une partie de mot : peut être un mot simple, un échappé, ou un guillemet
char *read_part(char *s, int *i)
{
    int start;

    if (s[*i] == '\'' || s[*i] == '"') // cas guillemet
        return read_quoted(s, i);

    if (s[*i] == '\\' && s[*i + 1]) // Cas caractère échappé
    {
        char *p = malloc(2);
        if (!p)
            return NULL;
        p[0] = s[*i + 1];
        p[1] = '\0';
        *i += 2; // avancer l'index de 2 caractères
        return p;
    }

    // Cas mot normal : lire jusqu'à espace ou symbole spécial
    start = *i;
    while (s[*i] && !isspace(s[*i]) && !strchr("|&;<>\"\'()", s[*i])
           && s[*i] != '\\')
        (*i)++;

    return strndup(s + start, *i - start); // dupliquer la sous-chaîne
}

// Lit un mot complet, en concaténant les parties (échappées ou guillemet)
char *read_word_extended(char *s, int *i)
{
    char *res = NULL;
    char *part;

    while (s[*i] && !isspace(s[*i]) && !strchr("|&;<>()", s[*i]))
    {
        part = read_part(s, i);
        if (!part)
            return free(res), NULL;
        res = append_part(res, part);
        free(part);
        if (!res)
            return NULL;
    }
    return res;
}

// Gestion des symboles spéciaux
int handle_symbol(char *s, int *i, token *tokens)
{
    // Vérifie et gère tous les symboles shell connus, simples et doubles
    if (s[*i] == '|' && s[*i + 1] == '|')
        return (*i += 2, add_token(tokens, TOK_OR, NULL));
    if (s[*i] == '|')
        return (*i += 1, add_token(tokens, TOK_PIPE, NULL));
    if (s[*i] == '&' && s[*i + 1] == '&')
        return (*i += 2, add_token(tokens, TOK_AND, NULL));
    if (s[*i] == '&')
        return (*i += 1, add_token(tokens, TOK_AMPERSAND, NULL));
    if (s[*i] == '>' && s[*i + 1] == '>')
        return (*i += 2, add_token(tokens, TOK_APPEND, NULL));
    if (s[*i] == '>' && s[*i + 1] == '&')
        return (*i += 2, add_token(tokens, TOK_DUP_OUT, NULL));
    if (s[*i] == '>' && s[*i + 1] == '|')
        return (*i += 2, add_token(tokens, TOK_REDIR_PIPE, NULL));
    if (s[*i] == '>')
        return (*i += 1, add_token(tokens, TOK_REDIR_OUT, NULL));
    if (s[*i] == '<' && s[*i + 1] == '<')
        return (*i += 2, add_token(tokens, TOK_HEREDOC, NULL));
    if (s[*i] == '<' && s[*i + 1] == '>')
        return (*i += 2, add_token(tokens, TOK_READ_WRITE, NULL));
    if (s[*i] == '<' && s[*i + 1] == '&')
        return (*i += 2, add_token(tokens, TOK_DUP_IN, NULL));
    if (s[*i] == '<')
        return (*i += 1, add_token(tokens, TOK_REDIR_IN, NULL));
    if (s[*i] == '(')
        return (*i += 1, add_token(tokens, TOK_PARO, NULL));
    if (s[*i] == ')')
        return (*i += 1, add_token(tokens, TOK_PARC, NULL));
    return 0; // aucun symbole trouvé
}

// Gère les caractères échappés (ex : \a) en créant un token
int handle_escape(char *s, int *i, token *tokens)
{
    char *word;

    if (s[*i] != '\\' || !s[*i + 1])
        return 0;

    word = malloc(2);
    if (!word)
        return 0;

    word[0] = s[*i + 1];
    word[1] = '\0';
    add_token(tokens, TOK_WORDS, word);
    *i += 2; // avancer après le caractère échappé
    return 1;
}

// Gère les tokens simples comme ';' ou '\n'
int handle_simple_token(char c, int *i, token *tokens)
{
    if (c == ';')
        return add_token(tokens, TOK_POINT_VIRGULE, NULL), (*i)++, 1;
    if (c == '\n')
        return add_token(tokens, TOK_NEWLINE, NULL), (*i)++, 1;
    return 0;
}

// Lecture de mots entre guillemets
char *read_quoted(char *input, int *i)
{
    char quote = input[*i];
    int start;
    int len = 0;
    char *word;

    (*i)++; // pour skip le premier
    start = *i;

    while (input[*i] && input[*i] != quote)
    {
        (*i)++;
        len++;
    }

    if (input[*i] != quote)
        return NULL; // non fermee = erreur a gerer plus tard

    word = malloc(len + 1);
    if (!word)
        return NULL;

    strncpy(word, input + start, len); // copier le contenu
    word[len] = '\0';

    (*i)++;
    return word;
}

// Gestion des commentaires
int skip_comment(char *s, int *i, int *simple_quote, int *double_quote)
{
    // Alterne l'état de citation si on rencontre un ' ou "
    if (s[*i] == '\'' && !*double_quote)
        *simple_quote = !*simple_quote;
    else if (s[*i] == '"' && !*simple_quote)
        *double_quote = !*double_quote;

    // Si pas dans une citation et # trouvé en début ou après espace =>
    // commentaire
    if (!*simple_quote && !*double_quote && s[*i] == '#'
        && (*i == 0 || isspace(s[*i - 1])))
    {
        while (s[*i] && s[*i] != '\n')
            (*i)++; // sauter tout jusqu'à fin de ligne
        return 1;
    }
    return 0;
}

// Détection des mots-clés shell
token_type check_keyword(char *word)
{
    if (strcmp(word, "if") == 0)
        return TOK_IF;
    if (strcmp(word, "then") == 0)
        return TOK_THEN;
    if (strcmp(word, "elif") == 0)
        return TOK_ELIF;
    if (strcmp(word, "else") == 0)
        return TOK_ELSE;
    if (strcmp(word, "fi") == 0)
        return TOK_FI;
    if (strcmp(word, "while") == 0)
        return TOK_WHILE;
    if (strcmp(word, "until") == 0)
        return TOK_UNTIL;
    if (strcmp(word, "for") == 0)
        return TOK_FOR;
    return TOK_WORDS; // si ce n'est pas un mot-clé, c'est un mot normal
}

// Fonction principale : lexer
token *lexer(char *input)
{
    int simple_quote = 0;
    int double_quote = 0;
    int i = 0;
    token *tokens = token_init(); // créer le token  qui sert a rien en tête
    if (!tokens)
        return NULL;

    while (input[i])
    {
        if (skip_comment(input, &i, &simple_quote, &double_quote))
            continue; // ignorer les commentaires
        else if (isspace(input[i]))
            i++; // ignorer les espaces
        else if (handle_escape(input, &i, tokens))
            continue; // gérer les caractères échappés
        else if (handle_symbol(input, &i, tokens))
            continue; // gérer les symboles spéciaux
        else if (handle_simple_token(input[i], &i, tokens))
            continue; // gérer ';' ou '\n'
        else if (input[i] == '\'' || input[i] == '"')
        {
            char *word = read_quoted(input, &i); // lire mot entre guillemets
            if (!word)
                return token_free_all(tokens), NULL; // erreur si non fermé
            add_token(tokens, TOK_WORDS, word);
        }
        else
        {
            char *word = read_word_extended(input, &i); // lire mot normal
            if (!word)
                return token_free_all(tokens), NULL;
            add_token(tokens, check_keyword(word),
                      word); // transformer en mot-clé si besoin
        }
    }
    if (tokens->next == NULL) // Si aucun token réel créé
    {
        token_free_all(tokens);
        return NULL;
    }

    return tokens; // retourne la liste chaînée de tokens
}
