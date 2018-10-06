/*
 * prom: a terminal/shell hijacker that extends a shell with extra
 * functionality. Copyright (C) 2018  Ahmed Alsharif
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
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