CC = gcc
CFLAGS = -Wall -Wextra -pedantic

SRC = main.c
PLUGINS = plugin.o
PLUGINS_SRC = plugin.c

plugin.so: plugin.o
	$(CC) -c -fPIC plugin.c
	$(CC) plugin.o -shared -fPIC -o plugin.so

clean: $(OBJ)
	rm -rf $(OBJ)
