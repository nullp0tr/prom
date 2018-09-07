#include "modules/mods.h"
#include "prom.h"
#include <string.h>

const char *module_add_usage = "usage: prom-module-add path\n";

static int cmd_arg_add(int argc, char *argv[]) {
    int exit_status = EXIT_SUCCESS;
    if (argc < 2) {
        usage_exit(module_add_usage);
    }

    const char *mod_path = argv[1];

    char *abs_mod_path = get_absolute_path(mod_path);
    if (abs_mod_path == NULL) {
        fprintf(stderr, "Error in translating mod path %s to absolute path.\n",
                mod_path);
        exit_status = EXIT_FAILURE;
        goto exit_proc;
    }

    struct module mod;
    const char modules_path[] = "";
    char *mod_err = load_module(abs_mod_path, &mod, modules_path);
    if (mod_err) {
        fprintf(stderr, "%s is not a valid module.\nError: %s.\n", mod_path,
                mod_err);
        exit_status = EXIT_FAILURE;
        goto clean_abs_mod_path;
    }

    err_int am = add_module(abs_mod_path, mod.name);
    if (am == -1) {
        fprintf(stderr, "Failed to add module from %s: ", mod_path);
        if (errno == EEXIST) {
            fprintf(stderr, "Plugin with name %s exists already.", mod.name);
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
    exit(exit_status);
}

const char *module_remove_usage = "usage: prom-module-remove name\n";

static int cmd_arg_remove(int argc, char *argv[]) {
    if (argc < 2) {
        usage_exit(module_remove_usage);
    }

    const char *mod_name = argv[1];

    if (rm_module(mod_name) == -1) {
        perror("Failed to remove module");
        return 1;
    }

    return 0;
}

static int cmd_arg_list(int argc, char *argv[]) {
    (void)argc, (void)argv;

    // char **modules = module_names();
    // for (char **mod = modules; *mod; mod++) {
    //     printf("%s\n", *mod);
    // }
    // free_module_names(modules);
    // return 0;

    struct module **mods = all_installed_modules("");
    for (struct module **mod = mods; *mod; mod++) {
        printf("-----\n");
        printf("Name: %s\n", (*mod)->name);
        printf("Desc: %s\n", (*mod)->desc);
    }
    printf("-----\n");

    free_installed_modules(mods);
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
