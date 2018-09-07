#include "prom.h"
#include <stdnoreturn.h>

noreturn void usage_exit(const char *usage) {
    fprintf(stderr, "%s", usage);
    exit(EXIT_FAILURE);
}
