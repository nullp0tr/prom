#define _XOPEN_SOURCE 600

#include "../filesystem/filesystem.h"
#include "../utils/error.h"
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

char *load_module(const char *path, struct module *mod,
                  const char *modules_path);

char *unload_module(struct module *mod);

err_int add_module(const char *path, const char *name);

err_int rm_module(const char *name);

char **module_names();
void free_module_names(char **module_names);

struct module **all_installed_modules(const char *modules_path);
void free_installed_modules(struct module **modules);