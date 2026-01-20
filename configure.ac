AC_INIT([42sh], [1.0], [paul.baudint@epita.fr])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_PROG_CC

AC_ARG_ENABLE([sanitize],
  AS_HELP_STRING([--enable-sanitize], [enable address sanitizer]),
  [enable_sanitize=$enableval],
  [enable_sanitize=no])

AS_IF([test "x$enable_sanitize" = "xyes"], [
  SANITIZE_CFLAGS="-fsanitize=address -fno-omit-frame-pointer"
  SANITIZE_LDFLAGS="-fsanitize=address"
  AC_SUBST([SANITIZE_CFLAGS])
  AC_SUBST([SANITIZE_LDFLAGS])
])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    tests/Makefile
])

AC_OUTPUT

