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

int error = 0;

ASTNode *parser(token *tokens)
{
    token *current = tokens->next;
    ASTNode *ast = parser_chain(&current);
    if (error)
    {
        free_ast(ast);
        error = 0;
        return NULL;
    }
    return ast;
}

ASTNode *parser_chain(token **current)
{
    ASTNode *left = parser_logique(current);

    while (*current && (*current)->type == TOK_POINT_VIRGULE)
    {
        *current = (*current)->next;

        ASTNode *right = parser_logique(current);

        ASTNode *node = new_node();
        node->type = AST_CHAIN;
        node->data.child.left = left;
        node->data.child.right = right;

        left = node;
    }

    return left;
}

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

ASTNode *parser_pipe(token **current)
{
    ASTNode *left = parser_redirect(current);
    if (!left)
        error = 1;

    while (*current && (*current)->type == TOK_PIPE)
    {
        *current = (*current)->next;

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

ASTNode *parser_redirect(token **current)
{
    ASTNode *next = parser_subshell(current);

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

        if (!*current || (*current)->type != TOK_WORDS)
        {
            fprintf(stderr, "pas de filename\n");
            return next;
        }

        ASTNode *node = new_node();
        node->type = AST_REDIRECT;
        node->data.redirect.mode = mode;
        // node->data.redirect.filename = (*current)->value;
        node->data.redirect.filename = strdup((*current)->value);

        node->data.redirect.cmd = next;

        *current = (*current)->next;
        next = node;
    }
    return next;
}

ASTNode *parser_subshell(token **current)
{
    if (*current && (*current)->type == TOK_PARO)
    {
        *current = (*current)->next;

        ASTNode *node = new_node();

        node->type = AST_SUBSHELL;
        node->data.subshell.child = parser_logique(current);

        if ((*current)->type != TOK_PARC || !node->data.subshell.child)
        {
            fprintf(stderr, "error not close parenthese\n");
            error = 1;
        }

        if (!node->data.subshell.child)
            error = 1;

        *current = (*current)->next;
        return node;
    }

    return parser_command(current);
}

ASTNode *parser_command(token **current)
{
    // regle IF
    if (*current && (*current)->type == TOK_IF)
        return parser_if(current);

    ASTNode *node = new_node();
    node->type = AST_COMMAND;

    size_t argc = 0;
    node->data.command.argv = NULL;

    while (*current && (*current)->type == TOK_WORDS)
    {
        node->data.command.argv =
            realloc(node->data.command.argv, (argc + 1) * sizeof(char *));
        node->data.command.argv[argc++] = (*current)->value;

        *current = (*current)->next;
    }

    if (!argc)
    {
        free(node);
        return NULL;
    }

    node->data.command.argv =
        realloc(node->data.command.argv, (argc + 1) * sizeof(char *));
    node->data.command.argv[argc] = NULL;

    node->data.command.len = argc;

    return node;
}
