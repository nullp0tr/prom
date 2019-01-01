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
#ifndef _MODS_H_
#define _MODS_H_

#define _XOPEN_SOURCE 600

#include "../dga/dga.h"
#include "../errm/errm.h"
#include "../filesystem/filesystem.h"
#include "api.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int module_main(int argc, char *argv[]);

struct module {
    void *dlhandle;
    const char *name;
    const char *desc;
    struct module_cbs cbs;
};

typedef char dlerr_char;

dlerr_char *load_module(const char *path, struct module *mod,
                        const char *modules_path);

dlerr_char *unload_module(struct module *mod);

typedef int err_int;
err_int add_module(const char *path, const char *name,
                   const char *modules_path);

err_int rm_module(const char *name, const char *modules_path);

char **mod_names(size_t *len_ret, const char *modules_path);
void free_mod_names(char **mod_names);

char **broken_mod_names();
void free_broken_mod_names();

struct module *installed_mods(size_t *len_ret, const char *modules_path);
void free_installed_mods(struct module *modules);

char **clean_ul_mod_names(size_t *len_ret, const char *modules_path);
void free_ul_mod_names(char **ul_mod_names);

#endif
