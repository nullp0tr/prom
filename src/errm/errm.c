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

#include "errm.h"
#include <assert.h>

const char *error_messages[] = {
    "",
    "Permission denied",
    "Not enough memory available",
    "Not the expected",
    "Does not exist",
    "Limit exceeded",
    "Parsing error",
    "Can't load (symbols from) a Dynamic Shared Oject",
    "An unkown error occured",
};

_Static_assert(sizeof(error_messages) == (ERROR_CODES_LEN) * sizeof(char *),
               "Missing error mesage. (lol, seriously)");

struct error_manager {
    enum error_codes err_code;
};

thread_local struct error_manager errm = {0};

void errm_error(enum error_codes err_code) { errm.err_code = err_code; }

void errm_clear() { errm.err_code = 0; }

int errm_error_code() { return errm.err_code; }

struct defer_mem_pool_t {
    char data[POOL_SIZE];
    size_t used;
};

struct defer_data_t {
    void (*static_defers[MAX_STATIC_DEFERS])(void *);
    void *static_defer_args[MAX_STATIC_DEFERS];
    void (*dyn_defers[MAX_DYN_DEFERS])(void *);
    void *dyn_defer_args[MAX_DYN_DEFERS];
    struct defer_mem_pool_t pool;
    size_t static_len;
    size_t dyn_len;
};

struct defer_stack_t {
    struct defer_data_t stack[STACK_SIZE];
    int curr;
};

thread_local struct defer_stack_t defer_stack = {.curr = -1};

int defer_push() {
    if (defer_stack.curr >= STACK_SIZE - 1)
        return 0;

    defer_stack.curr += 1;
    return 1;
}

int defer_pop() {
    if (defer_stack.curr <= -1)
        return 0;

    defer_stack.curr -= 1;
    return 1;
}

void static_defer(void (*handle)(void *), size_t s, void *arg) {
    struct defer_data_t *curr_stackframe = defer_stack.stack + defer_stack.curr;

    if (curr_stackframe->static_len >= MAX_STATIC_DEFERS) {
        abort();
    }

    if (curr_stackframe->pool.used + s > POOL_SIZE) {
        abort();
    }

    void *a = curr_stackframe->pool.data + curr_stackframe->pool.used;
    curr_stackframe->pool.used += s;

    memcpy(a, arg, s);

    curr_stackframe->static_defer_args[curr_stackframe->static_len] = a;
    curr_stackframe->static_defers[curr_stackframe->static_len] = handle;

    curr_stackframe->static_len += 1;
}

void defer(void (*handle)(void *), void *arg) {
    struct defer_data_t *curr_stackframe = defer_stack.stack + defer_stack.curr;

    if (curr_stackframe->dyn_len >= MAX_DYN_DEFERS) {
        abort();
    }

    curr_stackframe->dyn_defer_args[curr_stackframe->dyn_len] = arg;
    curr_stackframe->dyn_defers[curr_stackframe->dyn_len] = handle;

    curr_stackframe->dyn_len += 1;
}

void run_defers() {
    struct defer_data_t *curr_stackframe = defer_stack.stack + defer_stack.curr;

    while (curr_stackframe->static_len) {
        curr_stackframe->static_len -= 1;

        void (*handle)(void *) =
            curr_stackframe->static_defers[curr_stackframe->static_len];
        void *arg =
            curr_stackframe->static_defer_args[curr_stackframe->static_len];

        (*handle)(arg);
    }
    curr_stackframe->pool.used = 0;

    while (curr_stackframe->dyn_len) {
        curr_stackframe->dyn_len -= 1;

        void (*handle)(void *) =
            curr_stackframe->dyn_defers[curr_stackframe->dyn_len];
        void *arg = curr_stackframe->dyn_defer_args[curr_stackframe->dyn_len];

        (*handle)(arg);
    }
}

void errm_error_from_errno() {
    switch (errno) {
    case EACCES:
        errm_error(NO_PERMISSIONS);
        break;
    case ENFILE:
        errm_error(LIMIT_EXCEEDED);
        break;
    case ENOENT:
        errm_error(DOES_NOT_EXIST);
        break;
    case ENOTDIR:
        errm_error(NOT_THE_EXPECTED);
        break;
    case ENOMEM:
        errm_error(NOT_ENOUGH_MEMORY);
        break;
    }
}