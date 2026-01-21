#include "ast.h"

#include <stdlib.h>

ASTNode *new_node()
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
    }

    free(node);
}

void print_indent(int depth)
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

    case AST_SUBSHELL:
        printf("AST_SUBSHELL\n");
        break;

    case AST_IF:
        printf("IF\n");
        IfBranch *branch = node->data.ifnode.branches;
        while (branch)
        {
            print_indent(depth + 1);
            printf("CONDITION:\n");
            ast_pretty_print(branch->condition, depth + 2);
            print_indent(depth + 1);
            printf("THEN:\n");
            ast_pretty_print(branch->bloc, depth + 2);
            branch = branch->next;
        }
        if (node->data.ifnode.blocelse)
        {
            print_indent(depth + 1);
            printf("ELSE:\n");
            ast_pretty_print(node->data.ifnode.blocelse, depth + 2);
        }
        print_indent(depth);
        printf("FI\n");
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
