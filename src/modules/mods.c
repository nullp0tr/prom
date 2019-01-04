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
#include "mods.h"

static char *get_mod_path(const char *modules_path, const char *name) {
    errm_clear();

    size_t mod_path_len = strlen(modules_path) + strlen(name) + sizeof('\0');
    char *mod_path = malloc(mod_path_len);
    if (!mod_path) {
        errm_error(NOT_ENOUGH_MEMORY);
        return NULL;
    }

    snprintf(mod_path, mod_path_len, "%s%s", modules_path, name);
    return mod_path;
}

dlerr_char *load_module(const char *name, struct module *mod,
                        const char *modules_path) {
    errm_clear();

    char *dlerr = NULL;

    char *path = get_mod_path(modules_path, name);
    mod->dlhandle = dlopen(path, RTLD_NOW);
    free(path);
    dlerr = dlerror();
    if (dlerr) {
        errm_error(DYN_SHARED_OBJ_ERROR);
        return dlerr;
    }

    mod->name = dlsym(mod->dlhandle, "name");
    dlerr = dlerror();
    if (dlerr) {
        errm_error(DYN_SHARED_OBJ_ERROR);
        return dlerr;
    }

    mod->desc = dlsym(mod->dlhandle, "desc");
    dlerr = dlerror();
    if (dlerr) {
        errm_error(DYN_SHARED_OBJ_ERROR);
        return dlerr;
    }

    void (*init)(struct module_cbs *);
    *(void **)(&init) = dlsym(mod->dlhandle, "init");
    dlerr = dlerror();
    if (dlerr) {
        errm_error(DYN_SHARED_OBJ_ERROR);
        return dlerr;
    }

    (*init)(&mod->cbs);
    return NULL;
}

dlerr_char *unload_module(struct module *mod) {
    dlclose(mod->dlhandle);
    char *dlerr = dlerror();
    return dlerr;
}

err_int add_module(const char *path, const char *name,
                   const char *modules_path) {
    char *mod_path = get_mod_path(modules_path, name);
    int ret = symlink(path, mod_path);
    free(mod_path);
    return ret == -1 ? -1 : 0;
}

err_int rm_module(const char *name, const char *modules_path) {
    char *mod_path = get_mod_path(modules_path, name);
    int ret = remove(mod_path);
    free(mod_path);
    return ret == -1 ? -1 : 0;
}

char **mod_names(size_t *len_ret, const char *modules_path) {
    return dir_content(modules_path, len_ret);
}

void free_mod_names(char **names) { free_dir_content(names); }

/*
struct module *installed_mods(size_t *len_ret)
*/
edf_func2(struct module *, installed_mods, size_t *, len_ret, const char *,
          modules_path) {

    size_t len_mod_names;
    char **all_mod_names = mod_names(&len_mod_names, modules_path);
    on_error_return(NULL);
    defer((DeferHandle)free_mod_names, all_mod_names);

    struct module *all_mods = dga_new(0, struct module);
    if (!all_mods) {
        errm_error(NOT_ENOUGH_MEMORY);
        return NULL;
    }

    for (size_t i = 0; i < len_mod_names; i++) {
        char *mod_name = all_mod_names[i];

        dga_grow_or(all_mods, 1) {
            errm_error(NOT_ENOUGH_MEMORY);
            goto fail;
        }

        struct module curr_mod;
        load_module(mod_name, &curr_mod, modules_path);
        if (Errored) {
            goto fail;
        }

        all_mods[dga_len(all_mods) - 1] = curr_mod;
    }

    *len_ret = dga_len(all_mods);
    return all_mods;

fail:
    dga_free(all_mods);
    return NULL;
}

void free_installed_mods(struct module *mods) { dga_free(mods); }

char **clean_ul_mod_names(size_t *len_ret, const char *modules_path) {
    errm_clear();

    size_t len_names;
    char **names = mod_names(&len_names, modules_path);
    on_error_return(NULL);

    char **ul_names = dga_new(0, char *);
    if (!ul_names)
        errm_error(NOT_ENOUGH_MEMORY);

    for (size_t i = 0; i < len_names; i++) {
        char *name = names[i];

        struct module curr_mod;
        char *err = load_module(name, &curr_mod, modules_path);
        if (!err)
            continue;

        char *ul_mod_name = dga_new(strlen(name) + 1, 0);
        if (!ul_mod_name)
            goto memfail;

        strcpy(ul_mod_name, name);

        dga_grow_or(ul_names, 1) goto memfail;

        ul_names[dga_len(ul_names) - 1] = ul_mod_name;

        rm_module(name, modules_path);
    }
    free_mod_names(names);

    *len_ret = dga_len(ul_names);
    return ul_names;

memfail:
    free_ul_mod_names(ul_names);
    free_mod_names(names);
    errm_error(NOT_ENOUGH_MEMORY);
    return NULL;
}

void free_ul_mod_names(char **names) {
    char **name;
    dga_foreach(name, names) { dga_free(*name); }
    dga_free(names);
}
