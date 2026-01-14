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
    TOK_DO,
    TOK_DONE,
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
    TOK_APPEND,
    TOK_HEREDOC,
    TOK_PARO,
    TOK_PARC,
} token_type;

typedef struct token_struct
{
    token_type type;
    char *value;
    struct token_struct *next;
} token;

token *token_init(void);
token *token_new(token_type type, char *value);
void token_add(token *head, token *new);
void token_free_all(token *head);

token_type check_keyword(char *word);
int add_token(token *head, token_type type, char *value);
char *read_word(char *input, int *i);
int handle_symbol(char *s, int *i, token *tokens);
int handle_simple_token(char c, int *i, token *tokens);
int handle_escape(char *s, int *i, token *tokens);
char *read_quoted(char *input, int *i);

token *lexer(char *input_lx);

#endif // LEXER_H
