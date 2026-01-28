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
    AST_CHAIN,
    AST_IF,
    AST_FOR,
    AST_WHILE,
} ASTNodeType;

/*
Représente une branche d’un if ou elif
Chaque branche contient :
- une condition
- un bloc à exécuter si la condition est vraie
- un pointeur vers la branche suivante (elif)
*/

typedef struct IfBranch
{
    struct ASTNode *condition;
    struct ASTNode *bloc;
    struct IfBranch *next;
} IfBranch;

typedef struct ASTNode
{
    ASTNodeType type; // Type du node (commande, pipe, if, etc.)

    /*
    Union contenant les données spécifiques
    selon le type du node
    */

    union
    {
        struct // commande simple (cd,echo,...)
        {
            char **argv;
            int len;
        } command;

        struct // node binaire (pipe,...)
        {
            struct ASTNode *left;
            struct ASTNode *right;
        } child;

        struct // node redirect
        {
            struct ASTNode *cmd; // Commande associée à la redirection
            char *filename; // fichier ciblé
            token_type mode; // le token type dans lexer.h
        } redirect;

        struct // node sous shell
        {
            struct ASTNode *child;
        } subshell;

        /*
        Structure if
        Contient :
        - une liste de branches (if + elif)
        - un bloc else optionnel
        */

        struct
        {
            IfBranch *branches;
            struct ASTNode *blocelse;
        } ifnode;

        struct
        {
            char *var; // nom de la variable (ex: i)
            char **items; // liste des mots après "in"
            int items_len;
            struct ASTNode *body; // body du for
        } fornode;

        struct
        {
            struct ASTNode *condition;
            struct ASTNode *body;
            int is_until;  
        } whilenode;

    } data;
} ASTNode;

ASTNode *new_node(void);

/*
Affiche l’AST de manière lisible (debug)
depth = niveau d’indentation
*/
void ast_pretty_print(ASTNode *node, int depth);

void free_ast(
    ASTNode *node); // Libère récursivement toute la mémoire associée à un AST

#endif // AST_H
