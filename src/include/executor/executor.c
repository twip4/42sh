#include "executor.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../../common.h"
#include "../ast/ast.h"
#include "../builtins/builtins.h"
#include "../expand/expand.h"
#include "../garbage/garbage.h"
#include "../lexer/lexer.h"
#include "./redirects/redirects.h"

/*
Prototypes des fonctions d’exécution
Chaque fonction gère un type précis de node AST
*/
int executeur_de_commande(ASTNode *node);
int executeur_de_pipe(ASTNode *node);
int executeur_de_AND(ASTNode *node);
int executeur_de_OR(ASTNode *node);
int executeur_de_redirect(ASTNode *node);
int executeur_de_subshell(ASTNode *node);
int executeur_de_if(ASTNode *node);
int executeur_de_for(ASTNode *node);
int executeur_de_while(ASTNode *node);

extern ShellState g_shell;

static char **dup_argv(char **argv)
{
    int i;
    char **copy;

    for (i = 0; argv[i]; i++)
        ;

    copy = malloc(sizeof(char *) * (i + 1));
    if (!copy)
        return NULL;

    // Duplique chaque chaîne et ajoute au garbage
    for (i = 0; argv[i]; i++)
    {
        copy[i] = strdup(argv[i]);
        add_garbage(copy[i]); // chaque strdup est tracké
    }

    copy[i] = NULL;

    // Ajoute le tableau lui-même au garbage
    add_garbage(copy);

    return copy;
}

int l_executeur_d_ast(ASTNode *node)
{
    int status;
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
    case AST_FOR:
        return executeur_de_for(node);
    case AST_WHILE:
        return executeur_de_while(node);
    case AST_CHAIN:
        status = l_executeur_d_ast(node->data.child.left);

        if (g_shell.ctrl != CTRL_NONE)
            return status;

        status = l_executeur_d_ast(node->data.child.right);
        return status;
    default:
        fprintf(stderr, "l executeur  ast: type de noeud %d NOT IMPLEMENTED\n",
                node->type);
        return 1;
    }
}

int executeur_de_commande(ASTNode *node) // exécution de commande
{
    char **exec_argv;
    int len = node->data.command.len;

    // DUPLICATION AVANT EXPANSION
    exec_argv = dup_argv(node->data.command.argv);
    if (!exec_argv)
        return 1;

    // EXPANSION À L’EXÉCUTION
    expand_variable(exec_argv, len);
    // expand_variable(node->data.command.argv, node->data.command.len);

    int builtin_fait_maison = executeur_de_builtins(exec_argv);
    if (builtin_fait_maison != -1)
    {
        g_shell.last_return_code = builtin_fait_maison;
        return builtin_fait_maison;
    }

    pid_t pid = fork();

    if (pid == 0)
    {
        execvp(exec_argv[0],
               exec_argv); // plus tard utiliser execve
        perror("execvp");
        _exit(127); // dans la doc shell command not found
    }

    int status;
    waitpid(pid, &status, 0);
    g_shell.last_return_code = WEXITSTATUS(status);
    return g_shell.last_return_code;
}

int executeur_de_pipe(ASTNode *node) // exécution d’un pipe (cmd1 | cmd2)
{
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }
    // processus gauche : écrit dans le pipe

    pid_t left = fork();
    if (left == 0)
    {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        _exit(l_executeur_d_ast(node->data.child.left));
    }

    // processus droit : lit depuis le pipe
    pid_t right = fork();
    if (right == 0)
    {
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        _exit(l_executeur_d_ast(node->data.child.right));
    }

    // retour dans le processus parent

    close(fd[0]);
    close(fd[1]);

    int status;
    waitpid(left, &status, 0);
    waitpid(right, &status, 0);

    return WEXITSTATUS(status);
}

int executeur_de_AND(ASTNode *node) // exécution condition AND
{
    int status = l_executeur_d_ast(node->data.child.left);
    if (status == 0)
        return l_executeur_d_ast(node->data.child.right);
    return status;
}

int executeur_de_OR(ASTNode *node) // exécution condition OR
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
    {
        g_shell.is_subshell = 1;
        int ret = l_executeur_d_ast(node->data.subshell.child);

        free_ast(node->data.subshell.child);
        clear_garbage();
        token_free_all(g_shell.tokens); // si stockés globalement

        _exit(ret);
    }

    int status;
    waitpid(pid, &status, 0);
    // code exit du fork
    g_shell.last_return_code = WEXITSTATUS(status);
    return g_shell.last_return_code;
}

void restore_fds(int saved_stdin,
                 int saved_stdout) // permet de rebrancher correctement les
                                   // canaux stdin et stdout
{
    if (saved_stdin != -1)
    {
        dup2(saved_stdin,
             STDIN_FILENO); // redirige la sortie vers le stdin d'origine
        close(saved_stdin); // ferme l'ancien stdin
    }
    if (saved_stdout != -1)
    {
        dup2(saved_stdout, STDOUT_FILENO); // meme processus mais pour stdout
        close(saved_stdout);
    }
}

/*
fonction principale pour l'exécution des redirects
- applique la redirection
- exécute la commande
- restaure les fd
*/

int executeur_de_redirect(ASTNode *node)
{
    if (!node || node->type != AST_REDIRECT || !node->data.redirect.cmd)
        return 1;

    int saved_stdin = -1;
    int saved_stdout = -1;
    int result = 0;

    /*
    le switch redirige en fonction du type de redirect (définis dans le lexer)
    les fonctions redirect_input,... sont implementées dans le fichier
    redirect.c
    */

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

    if (result != 0) // En cas d’erreur, on restaure et on quitte
    {
        restore_fds(saved_stdin,
                    saved_stdout); // on utilise restore_fds pour restorer stdin
                                   // et stdout (modifiés pendant l'execution
                                   // des redirects)
        return 1;
    }

    int status =
        l_executeur_d_ast(node->data.redirect.cmd); // execute la commande
    restore_fds(
        saved_stdin,
        saved_stdout); // on utilise restore_fds pour restorer stdin et stdout
                       // (modifiés pendant l'execution des redirects)

    return status;
}

// exécution d’un if / elif / else

int executeur_de_if(ASTNode *node)
{
    IfBranch *branch = node->data.ifnode.branches;

    // Parcourt les branches if / elif
    while (branch)
    {
        int status = l_executeur_d_ast(branch->condition);

        if (status == 0)
            return l_executeur_d_ast(branch->bloc);

        branch = branch->next;
    }
    // exécution du else s’il existe
    if (node->data.ifnode.blocelse)
        return l_executeur_d_ast(node->data.ifnode.blocelse);

    return 0;
}

// execution de for
int executeur_de_for(ASTNode *node)
{
    if (!node || node->type != AST_FOR)
        return 1;

    int status = 0;

    g_shell.in_loop++;

    if (node->data.fornode.items_len == 0)
    {
        for (int i = 0; g_shell.argv && g_shell.argv[i]; i++)
        {
            setenv(node->data.fornode.var, g_shell.argv[i], 1);
            status = l_executeur_d_ast(node->data.fornode.body);

            if (g_shell.ctrl == CTRL_BREAK)
                break;

            if (g_shell.ctrl == CTRL_CONTINUE)
            {
                g_shell.ctrl = CTRL_NONE;
                continue;
            }
        }
    }
    else
    {
        for (int i = 0; i < node->data.fornode.items_len; i++)
        {
            setenv(node->data.fornode.var, node->data.fornode.items[i], 1);
            status = l_executeur_d_ast(node->data.fornode.body);

            if (g_shell.ctrl == CTRL_BREAK)
                break;

            if (g_shell.ctrl == CTRL_CONTINUE)
            {
                g_shell.ctrl = CTRL_NONE;
                continue;
            }
        }
    }

    if (g_shell.ctrl == CTRL_BREAK)
        g_shell.ctrl = CTRL_NONE;

    g_shell.in_loop--;

    return status;
}

int executeur_de_while(ASTNode *node)
{
    int last_body_status = 0;
    int body_executed = 0;

    g_shell.in_loop++;

    while (1)
    {
        int cond_status = l_executeur_d_ast(node->data.whilenode.condition);

        // while: continue if cond == 0
        // until: continue if cond != 0
        if ((!node->data.whilenode.is_until && cond_status != 0)
            || (node->data.whilenode.is_until && cond_status == 0))
            break;

        body_executed = 1;
        last_body_status = l_executeur_d_ast(node->data.whilenode.body);

        if (g_shell.ctrl == CTRL_BREAK)
            break;

        if (g_shell.ctrl == CTRL_CONTINUE)
        {
            g_shell.ctrl = CTRL_NONE;
            continue;
        }
    }

    if (g_shell.ctrl == CTRL_BREAK)
        g_shell.ctrl = CTRL_NONE;

    g_shell.in_loop--;
    if (body_executed)
    {
        return last_body_status;
    }
    return 0;
}
