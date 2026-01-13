#include "parser.h"

#include <stdlib.h>

ASTNode *parser_command(token **current);
ASTNode *parser_logique(token **current);
ASTNode *parser_pipe(token **current);
ASTNode *parser_redirect(token **current);

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
    ASTNode *left = parser_command(current);

    while (*current && (*current)->type == TOK_PIPE)
    {
        *current = (*current)->next;

        ASTNode *right = parser_command(current);

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
    (void)current;
    return NULL;
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
