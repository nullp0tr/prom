PACKAGE=prom
VERSION=0.0.1

AC_INIT([PACKAGE], [VERSION])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE([subdir-objects])

LT_INIT

PKG_CHECK_MODULES(CMOCKA, cmocka)
PKG_CHECK_MODULES(VTERM, vterm)
AC_SUBST(CMOCKA_CFLAGS)
AC_SUBST(CMOCKA_LIBS)
AC_SUBST(VTERM_CFLAGS)
AC_SUBST(VTERM_LIBS)

: ${CFLAGS="-Wall -Wextra -Wpedantic -std=c11 -g -fsanitize=address"}
AC_PROG_CC

AC_OUTPUT(Makefile src/Makefile src/filesystem/Makefile)
