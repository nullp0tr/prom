noinst_PROGRAMS = check_fs check_mods
TESTS = check_fs check_mods

LIBS = ../src/libprom.la -lncursesw -ldl -lvterm -lcmocka

check_fs_SOURCES = check_fs.c
check_fs_LDADD = $(LIBS)

check_mods_SOURCES = check_mods.c
check_mods_LDADD = $(LIBS)
