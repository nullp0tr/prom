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
