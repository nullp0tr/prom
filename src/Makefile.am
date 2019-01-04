bin_PROGRAMS = prom

AM_CFLAGS = $(CMOCKA_CFLAGS) $(VTERM_CFLAGS)

prom_SOURCES = main.c prom.c prom-module.c prom-shell.c prom.h modules/mods.c modules/mods.h modules/api.h filesystem/filesystem.c filesystem/filesystem.h dga/dga.c dga/dga.h libtracer/tracer.c libtracer/tracer.h errm/errm.c errm/errm.h shell/shell.c shell/shell.h shell/terminal.c shell/terminal.h shell/pty.c shell/pty.h
prom_LDADD = $(CMOCKA_LIBS) $(VTERM_LIBS) -ldl -lncursesw
