#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

typedef enum
{
    TOK_IF,
    TOK_THEN,
    TOK_ELIF,
    TOK_ELSE,
    TOK_FI,
    TOK_WHILE,
    TOK_FOR,
    TOK_POINT_VIRGULE,
    TOK_NEWLINE,
    TOK_APOSTROPHE,
    TOK_WORDS,

    TOK_PIPE,
    TOK_OR,
    TOK_AMPERSAND,
    TOK_AND,
    TOK_REDIR_IN,
    TOK_REDIR_OUT,
    TOK_REDIR_PIPE,
    TOK_APPEND,
    TOK_HEREDOC,
    TOK_DUP_OUT,
    TOK_DUP_IN,
    TOK_READ_WRITE,
    TOK_PARO,
    TOK_PARC,
} token_type;

typedef struct token_struct
{
    token_type type;
    char *value;
    struct token_struct *next;
} token;

void token_free_all(token *head);
token *lexer(char *input_lx);
char *read_quoted(char *input, int *i);

#endif // LEXER_H
