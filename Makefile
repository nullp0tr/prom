NAME = prom

CC = gcc
CFLAGS = -fsanitize=address,undefined -Wnonnull -fno-omit-frame-pointer -std=c11 -Wall -Wextra -g -pedantic
LDFLAGS = -lasan -lubsan -lncursesw -lutil -lutil -ldl
SOURCES = filesystem/filesystem.c main.c prom.c prom-shell.c \
prom-module.c modules/mods.c shell/shell.c shell/pty.c shell/terminal.c
LIBS = ./libvterm/.libs/libvterm.a
OBJS = $(SOURCES:.c=.o)
OBJS += ./libtracer/tracer.o

SUBDIRS = libtracer libvterm

CPPCHECK=cppcheck
CHECKFLAGS = -q --error-exitcode=1


main: $(OBJS) $(SUBDIRS)
	$(CC) -o $(NAME) $(OBJS) $(LIBS) $(LDFLAGS)

cppcheck.out: $(SOURCES)
	$(CPPCHECK) $(CHECKFLAGS) $^  >$@
	clang-tidy $^ --

format: $(SOURCES)
	clang-format -i $(SOURCES)

.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f $(SUBOBJS)
	rm -f $(NAME)

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

./libvterm/.libs/libvterm.a: ./shell/libvterm/%.c
	cd shell/libvterm && $(MAKE)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(LDFLAGS)
	$(CPPCHECK) $(CHECKFLAGS) $<
	clang-tidy $< --
