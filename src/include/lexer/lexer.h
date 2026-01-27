#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

typedef enum // liste tous les types de tokens
{
    TOK_IF, // "if"
    TOK_THEN, // "then"
    TOK_ELIF, // "elif"
    TOK_ELSE, // "else"
    TOK_FI, // "fi"
    TOK_WHILE, // "while"
    TOK_FOR, // "for"

    TOK_POINT_VIRGULE, // ";"
    TOK_NEWLINE, // saut de ligne "\n"
    TOK_APOSTROPHE, // "'" utilisé pour quotes (pas utilisé dans lexer.c)
    TOK_WORDS, // mots simples ou non-clés

    TOK_PIPE, // "|"
    TOK_OR, // "||"
    TOK_AMPERSAND, // "&"
    TOK_AND, // "&&"
    TOK_REDIR_IN, // "<"
    TOK_REDIR_OUT, // ">"
    TOK_REDIR_PIPE, // ">|"
    TOK_APPEND, // ">>"
    TOK_HEREDOC, // "<<"
    TOK_DUP_OUT, // ">&"
    TOK_DUP_IN, // "<&"
    TOK_READ_WRITE, // "<>"
    TOK_PARO, // "("
    TOK_PARC, // ")"
} token_type;

typedef struct token_struct // Définition de la structure token
{
    token_type type;
    char *value;
    struct token_struct *next;
} token;

void token_free_all(token *head);
token *lexer(char *input_lx);
char *read_quoted(char *input, int *i);

#endif // LEXER_H
