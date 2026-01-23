#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h> 

int redirect_input(const char *filename, int *saved_fd)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        perror("open input");
        return 1;
    }
    *saved_fd = dup(STDIN_FILENO);
    dup2(fd, STDIN_FILENO);
    close(fd);
    return 0;
}

int redirect_output(const char *filename, int *saved_fd)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("open output");
        return 1;
    }
    *saved_fd = dup(STDOUT_FILENO);
    dup2(fd, STDOUT_FILENO);
    close(fd);
    return 0;
}

int redirect_append(const char *filename, int *saved_fd)
{
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

int redirect_pipe(const char *data, int *saved_fd)
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return 1;
    }
    ssize_t written = write(pipefd[1], data, strlen(data));
    if (written == -1)
    {
        perror("write");
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }
    close(pipefd[1]);
    *saved_fd = dup(STDIN_FILENO);
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);
    return 0;
}

int redirect_dup_output(const char *fd_str, int *saved_fd)
{
    int target_fd = atoi(fd_str);
    *saved_fd = dup(STDOUT_FILENO);
    dup2(target_fd, STDOUT_FILENO);
    return 0;
}

int redirect_dup_input(const char *fd_str, int *saved_fd)
{
    int source_fd = atoi(fd_str);
    *saved_fd = dup(STDIN_FILENO);
    dup2(source_fd, STDIN_FILENO);
    return 0;
}

int redirect_read_write(const char *filename, int *saved_fd)
{
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd < 0)
    {
        perror("open read-write");
        return 1;
    }
    *saved_fd = dup(STDIN_FILENO);
    dup2(fd, STDIN_FILENO);
    close(fd);
    return 0;
}
