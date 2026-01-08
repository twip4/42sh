#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "lexer.h"

token *token_init(void)
{
    token *head;

    head = malloc(sizeof(token));
    if (!head)
        return NULL;
    head->type = -1;
    head->value = NULL;
    head->next = NULL;
    return head;
}

token *token_new(token_type type, char *value)
{
    token *t = malloc(sizeof(token));
    if (!t) return NULL;
    t->type = type;
    t->value = value;
    t->next = NULL;
    return t;
}

void token_add(token *head, token *new)
{
    token *tmp = head;

    while (tmp->next)
        tmp = tmp->next;
    tmp->next = new;
}

void token_free_all(token *head)
{
    token *tmp;
    token *current = head;

    while (current)
    {
        tmp = current->next;
        free(current->value);
        free(current);
        current = tmp;
    }
}

int add_token(token *head, token_type type, char *value)
{
    token *new = token_new(type, value);
    if (!new)
        return 0;
    token_add(head, new);
    return 1;
}

char *read_word(char *input, int *i)
{
    int start = *i;
    int len = 0;
    char *word;

    while (input[*i]
        && !isspace(input[*i])
        && !strchr("|&;<>", input[*i]))
    {
        (*i)++;
        len++;
    }

    word = malloc(len + 1);
    if (!word)
        return NULL;
    strncpy(word, input + start, len);
    word[len] = '\0';
    return word;
}

int handle_symbol(char *s, int *i, token *tokens)
{
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
    if (s[*i] == '>')
        return (*i += 1, add_token(tokens, TOK_REDIR_OUT, NULL));
    if (s[*i] == '<' && s[*i + 1] == '<')
        return (*i += 2, add_token(tokens, TOK_HEREDOC, NULL));
    if (s[*i] == '<')
        return (*i += 1, add_token(tokens, TOK_REDIR_IN, NULL));
    return 0;
}


token_type check_keyword(char *word)
{
    if (strcmp(word, "if") == 0) return TOK_IF;
    if (strcmp(word, "then") == 0) return TOK_THEN;
    if (strcmp(word, "elif") == 0) return TOK_ELIF;
    if (strcmp(word, "else") == 0) return TOK_ELSE;
    if (strcmp(word, "fi") == 0) return TOK_FI;
    return TOK_WORDS;
}

token *lexer(char *input_lx)
{
    int i = 0;
    token *tokens = token_init();
    if (!tokens)
        return NULL;

    while (input_lx[i])
    {
        if (isspace(input_lx[i]))
            i++;
        else if (handle_symbol(input_lx, &i, tokens))
            continue;
        else if (input_lx[i] == ';')
            add_token(tokens, TOK_POINT_VIRGULE, NULL), i++;
        else if (input_lx[i] == '\n')
            add_token(tokens, TOK_NEWLINE, NULL), i++;
        else
        {
            char *word = read_word(input_lx, &i);
            if (!word)
                return NULL;
            add_token(tokens, check_keyword(word), word);
        }
    }
    return tokens;
}
