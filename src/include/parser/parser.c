#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

ASTNode *parser_command(token **current);
ASTNode *parser_logique(token **current);
ASTNode *parser_pipe(token **current);
ASTNode *parser_redirect(token **current);
ASTNode *parser_subshell(token **current);

ASTNode *parser(token *tokens)
{
    token *current = tokens->next;
    return parser_logique(&current);
}

ASTNode *parser_logique(token **current)
{
    ASTNode *left = parser_pipe(current);

    while (*current
           && ((*current)->type == TOK_AND || (*current)->type == TOK_OR))
    {
        token_type tmp = (*current)->type;
        *current = (*current)->next;

        ASTNode *right = parser_pipe(current);

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

    while (*current && (*current)->type == TOK_PIPE)
    {
        *current = (*current)->next;

        ASTNode *right = parser_redirect(current);

        ASTNode *pipe = new_node();
        pipe->type = AST_PIPE;
        pipe->data.child.left = left;
        pipe->data.child.right = right;

        left = pipe;
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
               || (*current)->type == TOK_AMPERSAND))
    {
        *current = (*current)->next;

        ASTNode *node = new_node();
        node->type = AST_REDIRECT;
        node->data.redirect.mode = (*current)->type;
        node->data.redirect.cmd = node;
        node->data.redirect.filename = (*current)->value;

        if ((*current)->type != TOK_WORDS)
            printf("missing filename\n");

        *current = (*current)->next;
        next = node;
    }
    return next;
}

ASTNode *parser_subshell(token **current)
{
    if ((*current)->type == TOK_AMPERSAND)
    {
        *current = (*current)->next;

        ASTNode *node = new_node();

        node->type = AST_SUBSHELL;
        node->data.subshell.child = parser_logique(current);

        if ((*current)->type != TOK_AMPERSAND)
            printf("error not close parenthese\n");

        *current = (*current)->next;
        return node;
    }

    return parser_command(current);
}

ASTNode *parser_command(token **current)
{
    ASTNode *node = new_node();
    node->type = AST_COMMAND;
    node->data.command.argv = malloc(sizeof(void *));
    node->data.command.len = 1;

    while (*current && (*current)->type == TOK_WORDS)
    {
        node->data.command.argv = realloc(
            node->data.command.argv, node->data.command.len * sizeof(void *));
        node->data.command.argv[node->data.command.len++ - 1] =
            (*current)->value;

        *current = (*current)->next;
    }

    return node;
}
