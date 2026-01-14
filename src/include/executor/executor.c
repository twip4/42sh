#define _POSIX_C_SOURCE 200809L
#include "executor.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../ast/ast.h"
#include "../builtins/builtins.h"

static int executeur_de_commande(ASTNode *node);
static int executeur_de_pipe(ASTNode *node);
static int executeur_de_AND(ASTNode *node);
static int executeur_de_OR(ASTNode *node);
static int executeur_de_redirect(ASTNode *node);
static int executeur_de_subshell(ASTNode *node);

int l_executeur_d_ast(ASTNode *node)
{
    if (node == NULL)
        return 0;
    switch (node->type)
    {
    case AST_COMMAND:
        return executeur_de_commande(node);
    case AST_PIPE:
        return executeur_de_pipe(node);
    case AST_AND:
        return executeur_de_AND(node);
    case AST_OR:
        return executeur_de_OR(node);
    case AST_REDIRECT:
        return executeur_de_redirect(node);
    case AST_SUBSHELL:
        return executeur_de_subshell(node);
    default:
        fprintf(stderr, "l executeur  ast: type de noeud %d NOT IMPLEMENTED\n",
                node->type);
        return 1;
    }
}

static int executeur_de_commande(ASTNode *node)
{

    int builtin_fait_maison = executeur_de_builtins(node->data.command.argv);
    if(builtin_fait_maison != -1) return builtin_fait_maison;

    pid_t pid = fork();

    if (pid == 0)
    {
        execvp(node->data.command.argv[0],
               node->data.command.argv); // plus tard utiliser execve
        perror("execvp");
        _exit(127); // dans la doc shell command not found
    }

    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

static int executeur_de_pipe(ASTNode *node)
{
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }

    pid_t left = fork();
    if (left == 0)
    {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        _exit(l_executeur_d_ast(node->data.child.left));
    }

    pid_t right = fork();
    if (right == 0)
    {
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        _exit(l_executeur_d_ast(node->data.child.right));
    }

    close(fd[0]);
    close(fd[1]);

    int status;
    waitpid(left, &status, 0);
    waitpid(right, &status, 0);

    return WEXITSTATUS(status);
}

static int executeur_de_AND(ASTNode *node)
{
    int status = l_executeur_d_ast(node->data.child.left);
    if (status == 0)
        return l_executeur_d_ast(node->data.child.right);
    return status;
}

static int executeur_de_OR(ASTNode *node)
{
    int status = l_executeur_d_ast(node->data.child.left);
    if (status != 0)
        return l_executeur_d_ast(node->data.child.right);
    return status;
}

static int executeur_de_redirect(ASTNode *node)
{
    if (!node || node->type != AST_REDIRECT || !node->data.redirect.cmd) return 1;

    (void)node;
    fprintf(stderr, "REDIRECT: PAS FAIT !!!!\n");

    // Ouvrir le fichier cible (open)

    // Remplacer le descripteur standard (STDOUT_FILENO ou STDIN_FILENO) avec dup2

    // Fermer le fichier ouvert (le fd original)

    // Lancer la commande (builtin ou execvp)
    return 1;
}

static int executeur_de_subshell(ASTNode *node)
{
    pid_t pid = fork();
    if (pid == 0)
        _exit(l_executeur_d_ast(node->data.subshell.child));

    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}
