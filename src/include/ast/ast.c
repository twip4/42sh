#include "ast.h"

#include <stdlib.h>

ASTNode *new_node(void)
{
    ASTNode *new = malloc(sizeof(ASTNode));

    return new;
}

void free_if_branches(IfBranch *branch)
{
    IfBranch *next;

    while (branch)
    {
        next = branch->next;

        free_ast(branch->condition);
        free_ast(branch->bloc);
        free(branch);

        branch = next;
    }
}

void free_ast(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case AST_COMMAND:
        free(node->data.command.argv);
        break;

    case AST_PIPE:
    case AST_AND:
    case AST_OR:
    case AST_CHAIN:
        free_ast(node->data.child.left);
        free_ast(node->data.child.right);
        break;

    case AST_IF:
        free_if_branches(node->data.ifnode.branches);
        free_ast(node->data.ifnode.blocelse);
        break;

    case AST_REDIRECT:
        free_ast(node->data.redirect.cmd);
        free(node->data.redirect.filename);
        break;

    case AST_SUBSHELL:
        free_ast(node->data.subshell.child);
        break;
    default:
        break;
    }

    free(node);
}
