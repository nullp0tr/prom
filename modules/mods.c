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

const char modules_path[] = "/home/nullp0tr/.prom/modules/";

static char *get_mod_path(const char *modules_path, const char *name) {
    size_t mod_path_len = strlen(modules_path) + strlen(name) + 1;
    char *mod_path = malloc(mod_path_len);
    mod_path[0] = '\0';
    snprintf(mod_path, mod_path_len, "%s%s", modules_path, name);
    return mod_path;
}

dlerr_char *load_module(const char *name, struct module *mod,
                        const char *modules_path) {
    char *dlerr = NULL;

    char *path = get_mod_path(modules_path, name);
    mod->dlhandle = dlopen(path, RTLD_NOW);
    free(path);
    dlerr = dlerror();
    if (dlerr) {
        return dlerr;
    }

    mod->name = dlsym(mod->dlhandle, "name");
    dlerr = dlerror();
    if (dlerr) {
        return dlerr;
    }

    mod->desc = dlsym(mod->dlhandle, "desc");
    dlerr = dlerror();
    if (dlerr) {
        return dlerr;
    }

    void (*init)(struct module_cbs *);
    *(void **)(&init) = dlsym(mod->dlhandle, "init");
    dlerr = dlerror();
    if (dlerr) {
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

err_int add_module(const char *path, const char *name) {
    char *mod_path = get_mod_path(modules_path, name);
    int ret = symlink(path, mod_path);
    free(mod_path);
    return ret == -1 ? -1 : 0;
}

err_int rm_module(const char *name) {
    char *mod_path = get_mod_path(modules_path, name);
    int ret = remove(mod_path);
    free(mod_path);
    return ret == -1 ? -1 : 0;
}

char **mod_names() { return dir_content_names(modules_path); }

void free_mod_names(char **names) { free_dir_contents(names); }

struct module **installed_mods(const char *k) {
    char **all_mod_names = mod_names();
    if (!all_mod_names) {
        goto fail;
    }

    struct module **all_mods = malloc(sizeof(struct module *));
    if (all_mods == NULL) {
        goto memfail0;
    }

    size_t all_mods_len = 0;

    for (char **mod_name = all_mod_names; *mod_name; mod_name++) {
        struct module **temp_all_mods =
            realloc(all_mods, (++all_mods_len + 1) * sizeof(struct module *));
        if (temp_all_mods == NULL) {
            goto memfail1;
        }
        all_mods = temp_all_mods;

        struct module *curr_mod = malloc(sizeof(struct module));
        if (curr_mod == NULL) {
            goto memfail1;
        }

        all_mods[all_mods_len - 1] = curr_mod;

        char *err = load_module(*mod_name, curr_mod, modules_path);
        if (err) {
            free(curr_mod);
            all_mods_len--;
        }
    }

    free_mod_names(all_mod_names);
    all_mods[all_mods_len] = NULL;
    return all_mods;

memfail1:
    free_installed_mods(all_mods);
memfail0:
    free_mod_names(all_mod_names);
fail:
    return NULL;
}

void free_installed_mods(struct module **modules) {
    for (struct module **mod = modules; *mod; mod++) {
        free(*mod);
    }
    free(modules);
}

char **clean_ul_mod_names() {
    char **names = mod_names();
    if (!names) {
        return NULL;
    }

    char **ul_names = malloc(sizeof(char *));
    size_t ul_mods_len = 0;

    for (char **mod_name = names; *mod_name; mod_name++) {
        struct module curr_mod;
        char *err = load_module(*mod_name, &curr_mod, modules_path);
        if (err) {
            rm_module(*mod_name);
            char *ul_mod_name = malloc(sizeof(char) * strlen(*mod_name) + 1);
            ul_mod_name[0] = '\0';
            strcpy(ul_mod_name, *mod_name);

            char **temp =
                realloc(ul_names, sizeof(char *) * (++ul_mods_len + 1));
            if (!temp) {
                free(ul_mod_name);
                free_ul_mod_names(ul_names);
                ul_names = NULL;
                goto clean;
            }
            ul_names = temp;

            ul_names[ul_mods_len - 1] = ul_mod_name;
        }
    }

    ul_names[ul_mods_len] = NULL;

clean:
    free_mod_names(names);
    return ul_names;
}

void free_ul_mod_names(char **ul_names) {
    for (char **mod_name = ul_names; *mod_name; mod_name++) {
        free(*mod_name);
    }
    free(ul_names);
}