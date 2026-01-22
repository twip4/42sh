#ifndef REDIRECTS_H
#define REDIRECTS_H

#include "../../ast/ast.h"

static int redirect_input(const char *filename, int *saved_fd);
static int redirect_output(const char *filename, int *saved_fd);
static int redirect_append(const char *filename, int *saved_fd);
static int redirect_pipe(const char *data, int *saved_fd);
static int redirect_dup_output(const char *fd_str, int *saved_fd);
static int redirect_dup_input(const char *fd_str, int *saved_fd);
static int redirect_read_write(const char *filename, int *saved_fd);

#endif // REDIRECTS_H