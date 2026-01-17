#define _POSIX_C_SOURCE 200809L
#include "lexer.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    if (!t)
        return NULL;
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
        if (current->value)
        {
            free(current->value);
        }
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
    int len = 0;
    char *word;
    int j = *i;
    while (input[j] && !isspace(input[j]) && !strchr("|&;<>\"\'()", input[j]))
    {
        if (input[j] == '\\' && input[j + 1])
        {
            j += 2;
            len++;
        }
        else
        {
            j++;
            len++;
        }
    }
    word = malloc(len + 1);
    if (!word)
        return NULL;
    int k = 0;
    while (*i < j)
    {
        if (input[*i] == '\\' && input[*i + 1])
        {
            (*i)++;
            word[k++] = input[*i];
            (*i)++;
        }
        else
        {
            word[k++] = input[*i];
            (*i)++;
        }
    }
    word[k] = '\0';
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
    return 0;
}

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
    *i += 2;
    return 1;
}

int handle_simple_token(char c, int *i, token *tokens)
{
    if (c == ';')
        return add_token(tokens, TOK_POINT_VIRGULE, NULL), (*i)++, 1;
    if (c == '\n')
        return add_token(tokens, TOK_NEWLINE, NULL), (*i)++, 1;
    return 0;
}

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

    strncpy(word, input + start, len);
    word[len] = '\0';

    (*i)++;
    return word;
}

int skip_comment(char *s, int *i, int *simple_quote, int *double_quote)
{
    if (s[*i] == '\'' && !*double_quote)
        *simple_quote = !*simple_quote;
    else if (s[*i] == '"' && !*simple_quote)
        *double_quote = !*double_quote;

    if (!*simple_quote && !*double_quote && s[*i] == '#'
        && (*i == 0 || isspace(s[*i - 1])))
    {
        while (s[*i] && s[*i] != '\n')
            (*i)++;
        return 1;
    }
    return 0;
}

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
    if (strcmp(word, "do") == 0)
        return TOK_DO;
    if (strcmp(word, "done") == 0)
        return TOK_DONE;
    if (strcmp(word, "for") == 0)
        return TOK_FOR;
    return TOK_WORDS;
}

token *lexer(char *input)
{
    int simple_quote = 0;
    int double_quote = 0;
    int i = 0;
    token *tokens = token_init();
    if (!tokens)
        return NULL;

    while (input[i])
    {
        if (skip_comment(input, &i, &simple_quote, &double_quote))
            continue;
        else if (isspace(input[i]))
            i++;
        else if (handle_escape(input, &i, tokens))
            continue;
        else if (handle_symbol(input, &i, tokens))
            continue;
        else if (handle_simple_token(input[i], &i, tokens))
            continue;
        else if (input[i] == '\'' || input[i] == '"')
        {
            char *word = read_quoted(input, &i);
            if (!word)
                return token_free_all(tokens), NULL;
            add_token(tokens, TOK_WORDS, word);
        }
        else
        {
            char *word = read_word(input, &i);
            if (!word)
                return token_free_all(tokens), NULL;
            add_token(tokens, check_keyword(word), word);
        }
    }
    return tokens;
}
