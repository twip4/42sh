#include "ast.h"

#include <stdlib.h>

// Fonction pour créer un nouveau node AST
ASTNode *new_node(void)
{
    ASTNode *new = malloc(sizeof(ASTNode));

    return new; // Retourne le pointeur vers le nouveau nœud
}

// Fonction pour libérer une liste chaînée de branches "if"
void free_if_branches(IfBranch *branch)
{
    IfBranch *next;

    while (branch) // Parcours de toutes les branches
    {
        next = branch->next; // Sauvegarde du prochain élément avant de libérer

        free_ast(branch->condition); // Libération du node AST de la condition
        free_ast(
            branch
                ->bloc); // Libération du node AST du bloc associé à la branche
        free(branch); // Libération de la structure IfBranch elle-même

        branch = next;
    }
}

// Fonction récursive pour libérer tout l'arbre AST
void free_ast(ASTNode *node)
{
    if (!node)
        return;

    switch (
        node->type) // Selon le type du nœud, libérer les ressources spécifiques
    {
    // case AST_COMMAND:
    //     free(node->data.command
    //              .argv); // Libération du tableau d'arguments pour une
    //              commande
    //     break;
    case AST_COMMAND:
        if (node->data.command.argv)
        {
            for (size_t i = 0; node->data.command.argv[i]; i++)
                free(node->data.command.argv[i]);

            free(node->data.command.argv);
        }
        break;

    case AST_PIPE:
    case AST_AND:
    case AST_OR:
    case AST_CHAIN:
        free_ast(
            node->data.child.left); // Libération récursive du sous-arbre gauche
        free_ast(
            node->data.child.right); // Libération récursive du sous-arbre droit
        break;

    case AST_IF:
        free_if_branches(
            node->data.ifnode.branches); // Libération des branches if
        free_ast(
            node->data.ifnode.blocelse); // Libération du bloc "else" si présent
        break;

    case AST_FOR:
        free(node->data.fornode.var);

        for (int i = 0; i < node->data.fornode.items_len; i++)
            free(node->data.fornode.items[i]);
        free(node->data.fornode.items);

        free_ast(node->data.fornode.body);
        break;
    
    case AST_WHILE:
        free_ast(node->data.whilenode.condition);
        free_ast(node->data.whilenode.body);
        break;


    case AST_REDIRECT:
        free_ast(node->data.redirect.cmd);
        free(node->data.redirect.filename);
        break;

    case AST_SUBSHELL:
        free_ast(node->data.subshell
                     .child); // Libération récursive de l'arbre du sous-shell
        break;
    default:
        break;
    }

    free(node); // Enfin, libération du node lui-même
}
