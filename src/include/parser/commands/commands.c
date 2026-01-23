#define _POSIX_C_SOURCE 200809L
#include "commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int skip_semicolons_et_retour_a_la_ligne(token **current);

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

            if (!skip_semicolons_et_retour_a_la_ligne(current))
            {
                return NULL;
            }

            if (!*current || (*current)->type != TOK_THEN)
            {
                return NULL;
            }

            *current = (*current)->next;

            ASTNode *bloc = parser_logique(current);

            skip_semicolons_et_retour_a_la_ligne(current);

            IfBranch *branch = malloc(sizeof(IfBranch));
            branch->condition = condition;
            branch->bloc = bloc;
            branch->next = NULL;

            if (!node->data.ifnode.branches)
                node->data.ifnode.branches = branch;

            else
                last_branch->next = branch;

            last_branch = branch;
        }
        else if (t == TOK_ELSE)
        {
            *current = (*current)->next;
            node->data.ifnode.blocelse = parser_logique(current);

            skip_semicolons_et_retour_a_la_ligne(current);
        }
        else if (t == TOK_FI)
        {
            *current = (*current)->next;
            break;
        }
        else
            break;
    }
    return node;
}

int skip_semicolons_et_retour_a_la_ligne(token **current)
{
    int skipped = 0;
    while (*current && ((*current)->type == TOK_POINT_VIRGULE || (*current)->type == TOK_NEWLINE))
    {
        *current = (*current)->next;
        skipped = 1;
    }
    return skipped;
}

ASTNode *parser_for(token **current)
{
    (void)current;
    return NULL;
}

ASTNode *parser_while(token **current)
{
    (void)current;
    return NULL;
}