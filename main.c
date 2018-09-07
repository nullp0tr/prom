#include "prom.h"
#include <string.h>
#include <unistd.h>

const char *PROM_USAGE = "usage: prom <command> [--help]\n\
\n\
prom commands\n\
    module       Manage your modules\n\
    shell        Launch a prom shell\n";

int cmd_main(int argc, char *argv[]) {
    if (argc < 2) {
        usage_exit(PROM_USAGE);
    }

    if (strcmp(argv[1], "module") == 0) {
        return module_main(--argc, ++argv);
    } else if (strcmp(argv[1], "shell") == 0) {
        return shell_main(--argc, ++argv);
    }

    usage_exit(PROM_USAGE);
}

int main(int argc, char *argv[]) { return cmd_main(argc, argv); }
