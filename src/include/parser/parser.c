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

int is_keyword(token *tok, const char *kw)
{
    return tok && tok->type == TOK_WORDS && strcmp(tok->value, kw) == 0;
}

ASTNode *parser_chain_until(token **current, const char *stop)
{
    ASTNode *left = parser_logique(current);
    if (!left)
        return NULL;

    while (*current && !is_keyword(*current, stop)
           && (*current)->type == TOK_POINT_VIRGULE)
    {
        *current = (*current)->next; // consume ;

        if (is_keyword(*current, stop))
            break;

        ASTNode *right = parser_logique(current);

        ASTNode *node = new_node();
        node->type = AST_CHAIN;
        node->data.child.left = left;
        node->data.child.right = right;
        left = node;
    }

    return left;
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
ASTNode *parser_bracket_condition(token **current)
{
    if (!*current || strcmp((*current)->value, "[") != 0)
        return NULL;

    *current = (*current)->next; // consomme '['

    ASTNode *node = new_node();
    node->type = AST_COMMAND;
    node->data.command.argv = NULL;
    size_t argc = 0;

    // on ajoute tous les TOK_WORDS jusqu’au ']'
    while (*current && strcmp((*current)->value, "]") != 0)
    {
        char **tmp =
            realloc(node->data.command.argv, sizeof(char *) * (argc + 1));
        if (!tmp)
            return NULL;
        node->data.command.argv = tmp;
        node->data.command.argv[argc++] = strdup((*current)->value);
        *current = (*current)->next;
    }

    if (!*current || strcmp((*current)->value, "]") != 0)
    {
        fprintf(stderr, "syntax error: expected ']'\n");
        free_ast(node);
        return NULL;
    }

    // consomme ']'
    *current = (*current)->next;

    // termine argv
    char **tmp = realloc(node->data.command.argv, sizeof(char *) * (argc + 1));
    node->data.command.argv = tmp;
    node->data.command.argv[argc] = NULL;
    node->data.command.len = argc;

    return node;
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
    if (error)
    {
        free_ast(left);
        return NULL;
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

ASTNode *parser_command(token **current)
{
    // règle IF
    if (*current && (*current)->type == TOK_IF)
        return parser_if(current);

    // règle FOR
    if (*current && (*current)->type == TOK_FOR)
        return parser_for(current);

    // regle WHILE
    if (*current
        && ((*current)->type == TOK_WHILE || (*current)->type == TOK_UNTIL))
        return parser_while(current);

    ASTNode *node = new_node();
    node->type = AST_COMMAND;

    size_t argc = 0;
    node->data.command.argv = NULL;

    while (*current && (*current)->type == TOK_WORDS)
    {
        char **tmp =
            realloc(node->data.command.argv, sizeof(char *) * (argc + 1));
        if (!tmp)
            return NULL;

        node->data.command.argv = tmp;
        node->data.command.argv[argc++] = strdup((*current)->value);

        *current = (*current)->next;
    }

    if (!argc)
    {
        free(node);
        return NULL;
    }

    char **tmp = realloc(node->data.command.argv, sizeof(char *) * (argc + 1));
    if (!tmp)
        return NULL;

    node->data.command.argv = tmp;
    node->data.command.argv[argc] = NULL;
    node->data.command.len = argc;

    return node;
}
