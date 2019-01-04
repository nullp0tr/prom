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
#include "filesystem.h"
#include <errno.h>

static void *emalloc(size_t size);
edf_static_func1(void *, emalloc, size_t, size) {
    void *p = malloc(size);
    if (!p)
        errm_error(NOT_ENOUGH_MEMORY);
    return p;
}

/*
void *erealloc(void *ptr, size_t size)
*/
static void *ereallloc(void *ptr, size_t size);
edf_static_func2(void *, ereallloc, void *, ptr, size_t, size) {
    void *p = realloc(ptr, size);
    if (!p)
        errm_error(NOT_ENOUGH_MEMORY);
    return p;
}

/*
void *expand(const char *path)
*/
static void *expand(const char *path);
edf_static_func1(void *, expand, const char *, path) {

    wordexp_t w;
    if (wordexp(path, &w, 0)) {
        errm_error(PARSING_ERROR);
        return NULL;
    }
    static_defer((DeferHandle)wordfree, sizeof(wordexp_t), &w);

    char *ret = emalloc(strlen(w.we_wordv[0]) + 1);
    // cppcheck-suppress memleak
    on_error_return(NULL);

    strcpy(ret, w.we_wordv[0]);
    return ret;
}

static char *abs_path_from_relative(const char *relative_path);
edf_static_func1(char *, abs_path_from_relative, const char *, relative_path) {
    char *cwd = getcwd(NULL, 0);
    defer(free, cwd);

    size_t abs_path_len =
        strlen(relative_path) + sizeof('/') + strlen(cwd) + sizeof('\0');
    char *abs_path = emalloc(abs_path_len);
    // cppcheck-suppress memleak
    on_error_return(NULL);

    snprintf(abs_path, abs_path_len, "%s/%s", cwd, relative_path);
    return abs_path;
}

edf_func1(char *, abs_path, const char *, path) {
    char *translated_path = expand(path);
    on_error_return(NULL);

    if (translated_path[0] != '/') {
        defer(free, translated_path);
        return abs_path_from_relative(translated_path);
    }

    return translated_path;
}

/*
DIR *open_dir(const char *path)
*/
edf_static_func1(DIR *, open_dir, const char *, path) {
    char *expanded_path = expand(path);
    on_error_return(NULL);
    defer(free, expanded_path);

    DIR *dp = opendir(expanded_path);
    if (!dp)
        errm_error_from_errno();

    return dp;
}

static void close_dir(DIR *dp) { closedir(dp); }

void free_dir_content(char **content) {
    for (size_t i = 0; i < dga_len(content); i++) {
        dga_free(content[i]);
    }
    dga_free(content);
}

/*
char **dir_content(const char *path, size_t *len_ret)
*/
edf_func2(char **, dir_content, const char *, path, size_t *, len_ret) {
    DIR *dp = open_dir(path);
    on_error_return(NULL);
    defer((DeferHandle)close_dir, dp);

    *len_ret = 0;
    char **names = dga_new(0, char *);

    for (struct dirent *ep = readdir(dp); ep; ep = readdir(dp)) {
        const char *file_name = ep->d_name;
        if (file_name[0] == '.')
            continue;

        dga_grow_or(names, 1) goto memfail;

        char *name = dga_new(strlen(file_name) + sizeof('\0'), char);
        if (!name) {
            goto memfail;
        }

        strcpy(name, file_name);
        names[(*len_ret)++] = name;
    }

    return names;

memfail:
    errm_error(NOT_ENOUGH_MEMORY);
    free_dir_content(names);
    return NULL;
}
