#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./commands/commands.h"

ASTNode *parser_command(token **current);
ASTNode *parser_logique(token **current);
ASTNode *parser_pipe(token **current);
ASTNode *parser_redirect(token **current);
ASTNode *parser_subshell(token **current);
ASTNode *parser_chain(token **current);

/*
Flag global d’erreur de parsing
Permet d’annuler la construction de l’AST
*/
int error = 0;
// int error_code_sortie = 0; // 0 = ok, 2 = syntax error

/*
Point d’entrée du parser
Construit l’AST à partir de la liste de tokens
*/
ASTNode *parser(token *tokens)
{
    token *current = tokens->next; // On ignore le token HEAD
    ASTNode *ast =
        parser_chain(&current); // Parse la chaîne complète de commandes

    if (error) // En cas d’erreur, on libère l’AST
    {
        free_ast(ast);
        error = 0;
        return NULL;
    }
    return ast;
}

/*
Parse les chaînes de commandes séparées par ';'
Priorité la plus faible
*/
ASTNode *parser_chain(token **current)
{
    ASTNode *left = parser_logique(current);

    while (*current && (*current)->type == TOK_POINT_VIRGULE)
    {
        // Consomme le ';'
        *current = (*current)->next;

        ASTNode *right = parser_logique(current);

        // Création du nœud AST_CHAIN
        ASTNode *node = new_node();
        node->type = AST_CHAIN;
        node->data.child.left = left;
        node->data.child.right = right;

        left = node;
    }

    return left;
}

// Parse les opérateurs logiques && et ||
ASTNode *parser_logique(token **current)
{
    ASTNode *left = parser_pipe(current);
    if (!left)
        error = 1;

    while (*current
           && ((*current)->type == TOK_AND || (*current)->type == TOK_OR))
    {
        token_type tmp = (*current)->type;
        *current = (*current)->next;

        ASTNode *right = parser_pipe(current);
        if (!right)
            error = 1;

        ASTNode *node = new_node();
        node->type = (tmp == TOK_AND) ? AST_AND : AST_OR;
        node->data.child.left = left;
        node->data.child.right = right;

        left = node;
    }
    return left;
}

// Parse les pipes |
ASTNode *parser_pipe(token **current)
{
    ASTNode *left = parser_redirect(current);
    if (!left)
        error = 1;

    while (*current && (*current)->type == TOK_PIPE)
    {
        *current = (*current)->next; // Consomme le '|'

        ASTNode *right = parser_redirect(current);
        if (!right)
            error = 1;

        ASTNode *node = new_node();
        node->type = AST_PIPE;
        node->data.child.left = left;
        node->data.child.right = right;

        left = node;
    }
    return left;
}

// Parse les redirections
ASTNode *parser_redirect(token **current)
{
    ASTNode *next = parser_subshell(current);
    // La redirection s’applique à un sous-shell ou une commande

    while (*current
           && ((*current)->type == TOK_REDIR_IN
               || (*current)->type == TOK_REDIR_OUT
               || (*current)->type == TOK_APPEND
               || (*current)->type == TOK_DUP_OUT
               || (*current)->type == TOK_DUP_IN
               || (*current)->type == TOK_READ_WRITE
               || (*current)->type == TOK_REDIR_PIPE))
    {
        token_type mode = (*current)->type;
        *current = (*current)->next;

        // Une redirection doit être suivie d’un mot
        if (!*current || (*current)->type != TOK_WORDS)
        {
            fprintf(stderr, "pas de filename\n");
            return next;
        }
        // Création du node AST_REDIRECT
        ASTNode *node = new_node();
        node->type = AST_REDIRECT;
        node->data.redirect.mode = mode;
        // node->data.redirect.filename = (*current)->value;
        node->data.redirect.filename = strdup((*current)->value);
        node->data.redirect.cmd = next;

        *current = (*current)->next; // Consomme le filename

        next = node; // Permet les redirections en chaîne
    }
    return next;
}

// Parse un sous-shell
ASTNode *parser_subshell(token **current)
{
    if (*current && (*current)->type == TOK_PARO)
    {
        *current = (*current)->next; // Consomme '('

        ASTNode *node = new_node();

        node->type = AST_SUBSHELL;
        node->data.subshell.child = parser_chain(
            current); // Le contenu du sous-shell est une expression logique

        if ((*current)->type != TOK_PARC
            || !node->data.subshell.child) // Vérifie la parenthèse fermante
        {
            fprintf(stderr, "error not close parenthese\n");
            error = 1;
        }

        if (!node->data.subshell.child)
            error = 1;

        *current = (*current)->next; // Consomme ')'
        return node;
    }

    return parser_command(current);
}

// Parse une commande simple ou une structure if
ASTNode *parser_command(token **current)
{
    // regle IF
    if (*current && (*current)->type == TOK_IF)
        return parser_if(current);

    // regle FOR
    if (*current && (*current)->type == TOK_FOR)
        return parser_for(current);

    ASTNode *node = new_node();
    node->type = AST_COMMAND;

    size_t argc = 0;
    node->data.command.argv = NULL;

    while (*current
           && (*current)->type
               == TOK_WORDS) // Récupère tous les mots de la commande
    {
        node->data.command.argv =
            realloc(node->data.command.argv, (argc + 1) * sizeof(char *));
        node->data.command.argv[argc++] = (*current)->value;

        *current = (*current)->next;
    }

    if (!argc) // Pas de mots → pas une commande valide
    {
        free(node);
        return NULL;
    }
    // NULL-termination de argv
    node->data.command.argv =
        realloc(node->data.command.argv, (argc + 1) * sizeof(char *));
    node->data.command.argv[argc] = NULL;

    node->data.command.len = argc;

    return node;
}
