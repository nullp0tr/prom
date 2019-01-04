SUBDIRS = . filesystem

lib_LTLIBRARIES = libprom.la
libprom_la_SOURCES = $(SRCS)
include_HEADERS = prom.h

bin_PROGRAMS = prom

AM_CFLAGS = $(CMOCKA_CFLAGS) $(VTERM_CFLAGS)

SRCS = prom.c prom-module.c prom-shell.c prom.h modules/mods.c modules/mods.h modules/api.h filesystem/filesystem.c filesystem/filesystem.h dga/dga.c dga/dga.h libtracer/tracer.c libtracer/tracer.h errm/errm.c errm/errm.h shell/shell.c shell/shell.h shell/terminal.c shell/terminal.h shell/pty.c shell/pty.h
prom_SOURCES = main.c
prom_LDADD = libprom.la $(CMOCKA_LIBS) $(VTERM_LIBS) -ldl -lncursesw

# noinst_PROGRAMS = check_fs
# TESTS = check_fs
# check_fs_SOURCES = filesystem/check_fs.c $(SRCS)
# check_fs_LDADD = $(prom_LDADD)
