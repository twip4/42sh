AC_INIT([42sh], [1.0], [paul.baudint@epita.fr])

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC

CFLAGS="$CFLAGS -std=c99 -Wall -Wextra -Werror -pedantic"

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    tests/Makefile
])

AC_OUTPUT

