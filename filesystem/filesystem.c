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

char *get_absolute_path(const char *relative_path) {
    wordexp_t exp_result;
    wordexp(relative_path, &exp_result, 0);
    const char *translated_path = exp_result.we_wordv[0];
    size_t translated_path_len = strlen(translated_path);

    char *abs_path = NULL;

    if (translated_path[0] != '/') {
        char *cwd = getcwd(NULL, 0);
        size_t cwd_len = strlen(cwd);

        size_t slash_len = 1;
        size_t abs_path_len = translated_path_len + slash_len + cwd_len + 1;
        abs_path = malloc(abs_path_len);
        if (abs_path == NULL) {
            return NULL;
        }

        abs_path[0] = '\0';
        snprintf(abs_path, abs_path_len, "%s/", cwd);
        free(cwd);
    }

    else {
        abs_path = malloc(translated_path_len + 1);
        if (abs_path == NULL) {
            return NULL;
        }
        abs_path[0] = '\0';
    }

    strcat(abs_path, translated_path);
    wordfree(&exp_result);
    return abs_path;
}

char **dir_content_names(const char *path) {
    wordexp_t exp_result;
    if (wordexp(path, &exp_result, 0)) {
        return NULL;
    }
    const char *translated_path = exp_result.we_wordv[0];

    struct dirent *ep;
    DIR *dp = opendir(translated_path);

    if (dp == NULL) {
        return NULL;
    }

    char **files = NULL;
    size_t files_count = 0;
    while ((ep = readdir(dp))) {
        const char *file_name = ep->d_name;
        if (file_name[0] == '.') {
            continue;
        }
        size_t file_path_len = strlen(file_name) + 1;
        char *file_path = NULL;
        file_path = malloc(file_path_len * sizeof(char));
        file_path[0] = '\0';
        strcpy(file_path, file_name);

        char **temp = realloc(files, sizeof(char *) * (files_count + 1));
        if (!temp) {
            free(files);
            free(file_path);
            files = NULL;
            goto clean;
        }
        files = temp;

        files[files_count++] = file_path;
    }

    char **temp = realloc(files, sizeof(char *) * (files_count + 1));
    if (!temp) {
        free(files);
        files = NULL;
        goto clean;
    }
    files = temp;

    files[files_count] = NULL;

clean:
    closedir(dp);
    wordfree(&exp_result);
    return files;
}

char **dir_contents(const char *path) {
    char **files = NULL;
    size_t files_count = 0;

    wordexp_t exp_result;
    if (wordexp(path, &exp_result, 0)) {
        return NULL;
    }
    const char *translated_path = exp_result.we_wordv[0];

    struct dirent *ep;
    DIR *dp = opendir(translated_path);

    if (dp == NULL) {
        // error opening folder
        return NULL;
    }

    while ((ep = readdir(dp))) {
        const char *file_name = ep->d_name;
        if (file_name[0] == '.') {
            continue;
        }
        size_t file_path_len =
            strlen(translated_path) + 1 /* backslash */ + strlen(file_name) + 1;
        char *file_path = NULL;
        file_path = malloc(file_path_len * sizeof(char));
        file_path[0] = '\0';
        snprintf(file_path, file_path_len, "%s/%s", translated_path, file_name);

        files = realloc(files, sizeof(char *) * (files_count + 1));
        files[files_count++] = file_path;
    }
    closedir(dp);
    wordfree(&exp_result);
    files = realloc(files, sizeof(char *) * (files_count + 1));
    files[files_count] = NULL;
    return files;
}

void free_dir_contents(char **files) {
    foreach_file(file, files) { free(*file); }
    free(files);
}