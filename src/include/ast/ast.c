#include "ast.h"

#include <stdlib.h>

ASTNode *new_node()
{
    ASTNode *new = malloc(sizeof(ASTNode));

    return new;
}

static void print_indent(int depth)
{
    for (int i = 0; i < depth; i++)
        printf("  ");
}

void ast_pretty_print(ASTNode *node, int depth)
{
    if (!node)
        return;

    print_indent(depth);

    switch (node->type)
    {
    case AST_COMMAND:
        printf("COMMAND");
        if (node->data.command.argv)
        {
            printf(" [");
            for (int i = 0; node->data.command.argv[i]; i++)
                printf("%s%s", node->data.command.argv[i],
                       node->data.command.argv[i + 1] ? " " : "");
            printf("]");
        }
        printf("\n");
        break;

    case AST_PIPE:
        printf("PIPE\n");
        break;

    case AST_AND:
        printf("AND\n");
        break;

    case AST_OR:
        printf("OR\n");
        break;

    case AST_REDIRECT:
        printf("REDIR\n");
        break;

    default:
        printf("UNKNOWN\n");
    }

    if (node->type == AST_PIPE || node->type == AST_AND || node->type == AST_OR)
    {
        if (node->data.child.left)
            ast_pretty_print(node->data.child.left, depth + 1);
        if (node->data.child.right)
            ast_pretty_print(node->data.child.right, depth + 1);
    }
}
