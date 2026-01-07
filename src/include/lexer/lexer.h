#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

typedef enum {
    TOK_IF,
    TOK_THEN,
    TOK_ELIF,
    TOK_ELSE,
    TOK_FI,
    TOK_POINT_VIRGULE,
    TOK_NEWLINE,
    TOK_APOSTROPHE,
    TOK_WORDS
} token_type;

typedef struct {
    token_type type;
    char* value;
} token;


#endif //LEXER_H
