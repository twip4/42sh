#include "commands.h"

#include <stdlib.h>
#include <unistd.h>

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

ASTNode *parser_for(token **current) // parser for
{
    (void)current;
    return NULL;
}

ASTNode *parser_while(token **current) // parser while
{
    (void)current;
    return NULL;
}
