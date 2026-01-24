#include "executor.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../ast/ast.h"
#include "../builtins/builtins.h"
#include "../lexer/lexer.h"
#include "./redirects/redirects.h"

int executeur_de_commande(ASTNode *node);
int executeur_de_pipe(ASTNode *node);
int executeur_de_AND(ASTNode *node);
int executeur_de_OR(ASTNode *node);
int executeur_de_redirect(ASTNode *node);
int executeur_de_subshell(ASTNode *node);
int executeur_de_if(ASTNode *node);

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
    case AST_IF:
        return executeur_de_if(node);
    case AST_CHAIN:
        l_executeur_d_ast(node->data.child.left);
        return l_executeur_d_ast(node->data.child.right);
    default:
        fprintf(stderr, "l executeur  ast: type de noeud %d NOT IMPLEMENTED\n",
                node->type);
        return 1;
    }
}

int executeur_de_commande(ASTNode *node)
{
    int builtin_fait_maison = executeur_de_builtins(node->data.command.argv);
    if (builtin_fait_maison != -1)
        return builtin_fait_maison;

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

int executeur_de_pipe(ASTNode *node)
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

int executeur_de_AND(ASTNode *node)
{
    int status = l_executeur_d_ast(node->data.child.left);
    if (status == 0)
        return l_executeur_d_ast(node->data.child.right);
    return status;
}

int executeur_de_OR(ASTNode *node)
{
    int status = l_executeur_d_ast(node->data.child.left);
    if (status != 0)
        return l_executeur_d_ast(node->data.child.right);
    return status;
}

int executeur_de_subshell(ASTNode *node)
{
    pid_t pid = fork();
    if (pid == 0)
        _exit(l_executeur_d_ast(node->data.subshell.child));

    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

void restore_fds(int saved_stdin, int saved_stdout)
{
    if (saved_stdin != -1)
    {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }
    if (saved_stdout != -1)
    {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
}

int executeur_de_redirect(ASTNode *node)
{
    if (!node || node->type != AST_REDIRECT || !node->data.redirect.cmd)
        return 1;

    int saved_stdin = -1;
    int saved_stdout = -1;
    int result = 0;

    switch (node->data.redirect.mode)
    {
    case TOK_REDIR_IN:
        result = redirect_input(node->data.redirect.filename, &saved_stdin);
        break;
    case TOK_REDIR_OUT:
        result = redirect_output(node->data.redirect.filename, &saved_stdout);
        break;
    case TOK_APPEND:
        result = redirect_append(node->data.redirect.filename, &saved_stdout);
        break;
    case TOK_REDIR_PIPE:
        result = redirect_pipe(node->data.redirect.filename, &saved_stdin);
        break;
    case TOK_DUP_OUT:
        result =
            redirect_dup_output(node->data.redirect.filename, &saved_stdout);
        break;
    case TOK_DUP_IN:
        result = redirect_dup_input(node->data.redirect.filename, &saved_stdin);
        break;
    case TOK_READ_WRITE:
        result =
            redirect_read_write(node->data.redirect.filename, &saved_stdin);
        break;
    default:
        fprintf(stderr, "Redirection existe pas\n");
        return 1;
    }

    if (result != 0)
    {
        restore_fds(saved_stdin, saved_stdout);
        return 1;
    }

    int status = l_executeur_d_ast(node->data.redirect.cmd);
    restore_fds(saved_stdin, saved_stdout);

    return status;
}

int executeur_de_if(ASTNode *node)
{
    IfBranch *branch = node->data.ifnode.branches;

    while (branch)
    {
        int status = l_executeur_d_ast(branch->condition);

        if (status == 0)
            return l_executeur_d_ast(branch->bloc);

        branch = branch->next;
    }

    if (node->data.ifnode.blocelse)
        return l_executeur_d_ast(node->data.ifnode.blocelse);

    return 0;
}
