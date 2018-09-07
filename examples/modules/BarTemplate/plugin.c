#include "../../../modules/api.h"
#include <stdio.h>
#include <stdlib.h>

const char name[] = "BarTemplate";

const char desc[] = "A template for writing prom modules.";

//
int file_read_cb(const char *path);
int file_write_cb(const char *path);
//
void init(struct module_cbs *this) {
    this->read_file = &file_read_cb;
    this->write_file = &file_write_cb;
}

int file_read_cb(const char *path) {
    printf("plugin::read: %s\n", path);
    return 0;
}

int file_write_cb(const char *path) {
    printf("plugin::write: %s\n", path);
    return 0;
}
