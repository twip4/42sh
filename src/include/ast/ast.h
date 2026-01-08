#ifndef AST_H
#define AST_H

#include <stdio.h>

typedef enum {
    AST_COMMAND,      // ls -l
    AST_PIPE,         // |
    AST_AND,          // &&
    AST_OR,           // ||
    AST_REDIRECT,     // > < >>
    AST_SUBSHELL      // ( ... )
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;

    union {
        /* Commande simple */
        struct {
            char **argv;        // {"ls", "-l", NULL}
        } command;

        /* Pipe / && / || */
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
        } binary;

        /* Redirection */
        struct {
            struct ASTNode *cmd;
            char *filename;
            int mode;           // >, >>, <
        } redirect;

        /* Subshell */
        struct {
            struct ASTNode *child;
        } subshell;

    } data;

} ASTNode;





#endif //AST_H