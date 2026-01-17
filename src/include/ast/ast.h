#ifndef AST_H
#define AST_H

#include <stdio.h>
#include "../lexer/lexer.h"

typedef enum
{
    AST_COMMAND, // ls -l
    AST_PIPE, // |
    AST_AND, // &&
    AST_OR, // ||
    AST_REDIRECT, // > < >>
    AST_SUBSHELL, // ( ... )
    AST_IF  
} ASTNodeType;

typedef struct IfBranch
{
    struct ASTNode *condition;
    struct ASTNode *bloc;
    struct IfBranch *next;
} IfBranch;

typedef struct ASTNode
{
    ASTNodeType type;

    union
    {
        struct
        {
            char **argv;
            int len;
        } command;

        struct
        {
            struct ASTNode *left;
            struct ASTNode *right;
        } child;

        struct
        {
            struct ASTNode *cmd;
            char *filename;
            token_type mode;// le token type dans lexer.h
        } redirect;

        struct
        {
            struct ASTNode *child;
        } subshell;

        struct
        {
            IfBranch *branches;
            struct ASTNode *blocelse;
        } ifnode;

    } data;
} ASTNode;

ASTNode *new_node();
void ast_pretty_print(ASTNode *node, int depth);
void free_ast(ASTNode *node);

#endif // AST_H
