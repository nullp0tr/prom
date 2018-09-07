#include "mods.h"

const char modules_path[] = "/home/nullp0tr/.prom/modules/";

static char *get_mod_path(const char *modules_path, const char *name) {
    size_t mod_path_len = strlen(modules_path) + strlen(name) + 1;
    char *mod_path = malloc(mod_path_len);
    mod_path[0] = '\0';
    snprintf(mod_path, mod_path_len, "%s%s", modules_path, name);
    return mod_path;
}

char *load_module(const char *name, struct module *mod,
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

char *unload_module(struct module *mod) {
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

char **module_names() { return dir_content_names(modules_path); }

void free_module_names(char **names) { free_dir_contents(names); }

struct module **all_installed_modules(const char *k) {
    char **all_module_names = module_names();

    struct module **all_modules = malloc(sizeof(struct module *));
    size_t num_all_modules = 0;

    for (char **mod_name = all_module_names; *mod_name; mod_name++) {
        all_modules = realloc(all_modules, (++num_all_modules + 1) *
                                               sizeof(struct module *));

        struct module *curr_mod = malloc(sizeof(struct module));
        all_modules[num_all_modules - 1] = curr_mod;

        char *err = load_module(*mod_name, curr_mod, modules_path);
        if (err) {
            free(curr_mod);
            num_all_modules--;
        }
    }

    free_module_names(all_module_names);

    all_modules[num_all_modules] = NULL;

    return all_modules;
}

void free_installed_modules(struct module **modules) {
    for (struct module **mod = modules; *mod; mod++) {
        free(*mod);
    }
    free(modules);
}
