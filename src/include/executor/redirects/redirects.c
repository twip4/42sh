#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
Redirection de l’entrée standard (stdin)
Exemple : cmd < fichier
*/
int redirect_input(const char *filename, int *saved_fd)
{
    int fd = open(filename, O_RDONLY); // Ouvre le fichier en lecture seule
    if (fd < 0)
    {
        perror("open input");
        return 1;
    }
    *saved_fd = dup(STDIN_FILENO); // Sauvegarde l’ancien stdin
    dup2(fd, STDIN_FILENO); // Redirige stdin vers le fichier
    close(fd);
    return 0;
}

/*
Redirection de la sortie standard (stdout)
Exemple : cmd > fichier
*/
int redirect_output(const char *filename, int *saved_fd)
{
    // Ouvre ou crée le fichier, écrase le contenu
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("open output");
        return 1;
    }
    *saved_fd = dup(STDOUT_FILENO); // Sauvegarde l’ancien stdout
    dup2(fd, STDOUT_FILENO); // Redirige stdout vers le fichier
    close(fd);
    return 0;
}

/*
Redirection en mode ajout (append)
Exemple : cmd >> fichier
*/
int redirect_append(const char *filename, int *saved_fd)
{
    // Ouvre ou crée le fichier en ajout
    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0)
    {
        perror("open append");
        return 1;
    }
    *saved_fd = dup(STDOUT_FILENO);
    dup2(fd, STDOUT_FILENO);
    close(fd);
    return 0;
}

/*
Redirection via un pipe interne
Permet de fournir une chaîne comme entrée standard
*/
int redirect_pipe(const char *data, int *saved_fd)
{
    int pipefd[2];
    // Création du pipe
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return 1;
    }
    // Écriture des données dans le pipe
    ssize_t written = write(pipefd[1], data, strlen(data));
    if (written == -1)
    {
        perror("write");
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }
    close(pipefd[1]);

    *saved_fd = dup(STDIN_FILENO); // Sauvegarde de l’ancien stdin
    dup2(pipefd[0],
         STDIN_FILENO); // Redirige stdin vers l’extrémité lecture du pipe
    close(pipefd[0]);
    return 0;
}

/*
Duplication de descripteur de sortie
Exemple : cmd 1>&2
*/

int redirect_dup_output(const char *fd_str, int *saved_fd)
{
    int target_fd = atoi(fd_str); // Conversion de la chaîne vers un fd
    *saved_fd = dup(STDOUT_FILENO);
    dup2(target_fd, STDOUT_FILENO); // Duplique le fd cible vers stdout
    return 0;
}

/*
Duplication de descripteur d’entrée
Exemple : cmd 0<&1
*/

int redirect_dup_input(const char *fd_str, int *saved_fd)
{
    int source_fd = atoi(fd_str);
    *saved_fd = dup(STDIN_FILENO);
    dup2(source_fd, STDIN_FILENO); // Duplique le fd source vers stdin
    return 0;
}

/*
Redirection lecture + écriture
Exemple : cmd <> fichier
*/
int redirect_read_write(const char *filename, int *saved_fd)
{
    int fd = open(filename, O_RDWR | O_CREAT,
                  0644); // Ouvre ou crée le fichier en lecture et écriture
    if (fd < 0)
    {
        perror("open read-write");
        return 1;
    }
    *saved_fd = dup(STDIN_FILENO);
    dup2(fd, STDIN_FILENO); // Redirige stdin vers le fichier
    close(fd);
    return 0;
}
