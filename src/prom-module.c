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
#include "modules/mods.h"
#include "prom.h"
#include <stdbool.h>
#include <string.h>

const char *modules_path = "/home/nullp0tr/.prom/modules/";

const char *module_add_usage = "usage: prom-module-add path\n";

static int cmd_arg_add(int argc, char *argv[]) {
    int exit_status = EXIT_SUCCESS;
    if (argc < 2) {
        usage_exit(module_add_usage);
    }

    const char *mod_path = argv[1];

    char *abs_mod_path = abs_path(mod_path);
    if (!abs_mod_path) {
        exit_status = EXIT_FAILURE;
        goto exit_proc;
    }

    struct module mod;
    const char modules_path[] = "";
    char *mod_err = load_module(abs_mod_path, &mod, modules_path);
    if (mod_err) {
        fprintf(stderr, "%s is not a valid module.\n%s.\n", mod_path, mod_err);
        exit_status = EXIT_FAILURE;
        goto clean_abs_mod_path;
    }

    err_int am = add_module(abs_mod_path, mod.name, modules_path);
    if (am == -1) {
        fprintf(stderr, "Failed to add module from %s: ", mod_path);
        if (errno == EEXIST) {
            fprintf(stderr, "Module with name %s exists already.", mod.name);
        } else {
            fprintf(stderr, "%s", strerror(errno));
        }
        fprintf(stderr, "\n");
        exit_status = EXIT_FAILURE;
        goto unload_mod;
    }

unload_mod:
    unload_module(&mod);
clean_abs_mod_path:
    free(abs_mod_path);
exit_proc:
    return exit_status;
}

const char *module_remove_usage = "usage: prom-module-remove name\n";

static int cmd_arg_remove(int argc, char *argv[]) {
    if (argc < 2) {
        usage_exit(module_remove_usage);
    }

    const char *mod_name = argv[1];

    err_int rm = rm_module(mod_name, modules_path);
    if (rm == -1) {
        if (errno == ENOENT) {
            fprintf(stderr, "Module with name %s doesn't exist.\n", mod_name);
        } else {
            fprintf(stderr, "%s. %d\n", strerror(errno), errno);
        }
        return 1;
    }

    return 0;
}

static int cmd_arg_list(int argc, char *argv[]) {
    (void)argc, (void)argv;
    bool f_clean = false;
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--clean"))
            f_clean = true;
    }

    if (f_clean) {
        size_t damaged_len;
        char **damaged = clean_ul_mod_names(&damaged_len, modules_path);
        if (!damaged) {
            return -1;
        }

        for (size_t i = 0; i < damaged_len; i++) {
            char *name = damaged[i];
            printf("Removed: %s\n", name);
        }

        free_ul_mod_names(damaged);
        return 0;
    }

    size_t len_ret = 0;
    struct module *mods = installed_mods(&len_ret, modules_path);
    if (!mods) {
        return 1;
    }

    for (size_t i = 0; i < len_ret; i++) {
        printf("-----\n");
        printf("Name: %s\n", mods[i].name);
        printf("Desc: %s\n", mods[i].desc);
    }
    free_installed_mods(mods);

    printf("-----\n");
    return 0;
}

const char *module_usage = "usage: prom-module <command>\n\n"
                           "commands:\n"
                           "    list      list all modules\n"
                           "    add       add a new namespace\n"
                           "    remove    remove an existing namespace\n";

int module_main(int argc, char *argv[]) {
    if (argc < 2) {
        usage_exit(module_usage);
    }

    const char *cmd_arg = argv[1];
    int (*cmd)(int, char **);

    if (!strcmp(cmd_arg, "list")) {
        cmd = &cmd_arg_list;
    }

    else if (!strcmp(cmd_arg, "add")) {
        cmd = &cmd_arg_add;
    }

    else if (!strcmp(cmd_arg, "remove")) {
        cmd = &cmd_arg_remove;
    }

    else {
        usage_exit(module_usage);
    }

    return (*cmd)(--argc, ++argv);
}
