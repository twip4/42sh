#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

ASTNode *parser_command(token **current);
ASTNode *parser_logique(token **current);
ASTNode *parser_pipe(token **current);
ASTNode *parser_redirect(token **current);
ASTNode *parser_subshell(token **current);
ASTNode *parser_if(token **current);

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
    if ((*current)->type == TOK_PARO)
    {
        *current = (*current)->next;

        ASTNode *node = new_node();

        node->type = AST_SUBSHELL;
        node->data.subshell.child = parser_logique(current);

        if ((*current)->type != TOK_PARC)
            printf("error not close parenthese\n");

        *current = (*current)->next;
        return node;
    }

    return parser_command(current);
}

ASTNode *parser_command(token **current)
{
    //regle IF
    if (*current && (*current)->type == TOK_IF) return parser_if(current);

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

    // tu mets pas de null a la fin gros con, ya que des segmentations fault
    node->data.command.argv =
        realloc(node->data.command.argv, (argc + 1) * sizeof(char *));
    node->data.command.argv[argc] = NULL;

    node->data.command.len = argc;

    return node;
}

ASTNode *parser_if(token **current)
{
    if (!current || !*current || (*current)->type != TOK_IF)
        return NULL;

    ASTNode *node = new_node();
    node->type = AST_IF;
    node->data.ifnode.branches = NULL;
    node->data.ifnode.blocelse = NULL;

    IfBranch *last_branch = NULL;

    while (*current)
    {
        token_type t = (*current)->type;

        if (t == TOK_IF || t == TOK_ELIF)
        {
            *current = (*current)->next;

            ASTNode *condition = parser_logique(current);

            while (*current && (*current)->type == TOK_POINT_VIRGULE)
            {
                *current = (*current)->next;
            }

            if (!*current || (*current)->type != TOK_THEN)
            {
                printf("parser_if: ya pas THEN\n");
                return NULL;
            }
            *current = (*current)->next;

            ASTNode *bloc = parser_logique(current);

            while (*current && (*current)->type == TOK_POINT_VIRGULE)
            {
                *current = (*current)->next;
            }

            IfBranch *branch = malloc(sizeof(IfBranch));
            branch->condition = condition;
            branch->bloc = bloc;
            branch->next = NULL;

            if (!node->data.ifnode.branches){
                node->data.ifnode.branches = branch;
            }
            else last_branch->next = branch;

            last_branch = branch;
        }
        else if (t == TOK_ELSE)
        {
            *current = (*current)->next;
            node->data.ifnode.blocelse = parser_logique(current);

            while (*current && (*current)->type == TOK_POINT_VIRGULE)
            {
                *current = (*current)->next;
            }
        }
        else if (t == TOK_FI)
        {
            *current = (*current)->next;
            break;
        }
        else
        {
            printf("parser_if: token inattendu %d (value=%s)\n", t,(*current)->value ? (*current)->value : "NULL");
            break;
        }
    }

    return node;
}