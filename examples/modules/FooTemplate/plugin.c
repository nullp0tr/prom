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
#include "../../../modules/api.h"
#include <stdio.h>
#include <stdlib.h>

const char name[] = "FooTemplate";

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
