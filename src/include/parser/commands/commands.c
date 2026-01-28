#include "commands.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int error;

int skip_semicolons_et_retour_a_la_ligne(token **current);

/*
parse une structure conditionnelle if / elif / else / fi
et construit le nœud AST correspondant
*/

ASTNode *parser_if(token **current) // parser pour l'instruction IF
{
    // vérification de base : token valide et début par IF
    if (!current || !*current || (*current)->type != TOK_IF)
        return NULL;

    // création du node  de type AST_IF
    ASTNode *node = new_node();
    node->type = AST_IF;
    node->data.ifnode.branches = NULL;
    node->data.ifnode.blocelse = NULL;

    // sert à chaîner les branches if / elif
    IfBranch *last_branch = NULL;

    while (*current) // boucle gère successivement if, elif, else et fi
    {
        token_type t = (*current)->type;

        if (t == TOK_IF || t == TOK_ELIF) // gestion des branches IF et ELIF
        {
            *current = (*current)->next;

            ASTNode *condition = parser_logique(current);

            /*
            Après la condition, on attend des séparateurs
            (newline ou ;)
            */

            if (!skip_semicolons_et_retour_a_la_ligne(current))
            {
                return NULL;
            }
            // Le THEN est obligatoire
            if (!*current || (*current)->type != TOK_THEN)
            {
                return NULL;
            }

            *current = (*current)->next; // consomme THEN

            ASTNode *bloc = parser_logique(current); // parse le bloc condition

            skip_semicolons_et_retour_a_la_ligne(
                current); // ignore les separateurs

            // création d’une nouvelle branche if / elif

            IfBranch *branch = malloc(sizeof(IfBranch));
            branch->condition = condition;
            branch->bloc = bloc;
            branch->next = NULL;

            // ajout de la branche à la liste
            if (!node->data.ifnode.branches)
                node->data.ifnode.branches = branch;

            else
                last_branch->next = branch;

            last_branch = branch;
        }
        else if (t == TOK_ELSE) // Gestion du ELSE
        {
            *current = (*current)->next; // Consomme ELSE
            node->data.ifnode.blocelse =
                parser_logique(current); // Parse le bloc else

            skip_semicolons_et_retour_a_la_ligne(
                current); // ignore les separateurs apres le bloc else
        }
        else if (t == TOK_FI)
        {
            *current = (*current)->next; // Consomme FI et sort
            break;
        }
        else
            break; // Token inattendu : sortie de la boucle
    }
    return node;
}

/*
Avance le pointeur de tokens tant que l’on rencontre
des points-virgules ou des retours à la ligne
Retourne 1 si au moins un token a été ignoré, sinon 0
*/
int skip_semicolons_et_retour_a_la_ligne(token **current)
{
    int skipped = 0;
    while (*current
           && ((*current)->type == TOK_POINT_VIRGULE
               || (*current)->type == TOK_NEWLINE))
    {
        *current = (*current)->next;
        skipped = 1;
    }
    return skipped;
}

// parser for
ASTNode *parser_for(token **current)
{
    if (!current || !*current || (*current)->type != TOK_FOR)
        return NULL;

    // consume "for"
    *current = (*current)->next;

    // variable name
    if (!*current || (*current)->type != TOK_WORDS)
    {
        fprintf(stderr, "for: expected variable name\n");
        error = 1;
        return NULL;
    }

    ASTNode *node = new_node();
    node->type = AST_FOR;

    node->data.fornode.var = strdup((*current)->value);
    add_garbage(node->data.fornode.var);
    node->data.fornode.items = NULL;
    node->data.fornode.items_len = 0;

    *current = (*current)->next;

    /* optional: in WORD ... */
    if (*current && (*current)->type == TOK_WORDS
        && strcmp((*current)->value, "in") == 0)
    {
        *current = (*current)->next;

        while (*current && (*current)->type == TOK_WORDS)
        {
            node->data.fornode.items =
                realloc(node->data.fornode.items,
                        sizeof(char *) * (node->data.fornode.items_len + 1));
            if (!node->data.fornode.items)
                exit(1);

            node->data.fornode.items[node->data.fornode.items_len++] =
                strdup((*current)->value);
            add_garbage(
                node->data.fornode.items[node->data.fornode.items_len - 1]);

            *current = (*current)->next;
        }

        if (node->data.fornode.items)
            add_garbage(node->data.fornode.items);
    }

    // expect ; or newline
    if (!skip_semicolons_et_retour_a_la_ligne(current))
    {
        fprintf(stderr, "for: expected newline or ;\n");
        error = 1;
        return NULL;
    }

    // expect "do"
    if (!*current || (*current)->type != TOK_WORDS
        || strcmp((*current)->value, "do") != 0)
    {
        fprintf(stderr, "for: expected do\n");
        error = 1;
        return NULL;
    }

    *current = (*current)->next;

    // body
    node->data.fornode.body = parser_logique(current);
    if (!node->data.fornode.body)
    {
        error = 1;
        return NULL;
    }

    skip_semicolons_et_retour_a_la_ligne(current);

    // expect "done"
    if (!*current || (*current)->type != TOK_WORDS
        || strcmp((*current)->value, "done") != 0)
    {
        fprintf(stderr, "for: expected done\n");
        error = 1;
        return NULL;
    }

    *current = (*current)->next;

    return node;
}
