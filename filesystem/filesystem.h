#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wordexp.h>

#define foreach_file(var, files) for (char **var = files; *var; var++)

char *get_absolute_path(const char *relative_path);

char **dir_contents(const char *path);
char **dir_content_names(const char *path);
void free_dir_contents(char **files);

#endif