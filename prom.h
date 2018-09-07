#ifndef PROM_H
#define PROM_H
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

noreturn void usage_exit(const char *usage);

int shell_main(int argc, char *argv[]);

int module_main(int argc, char *argv[]);

#endif
