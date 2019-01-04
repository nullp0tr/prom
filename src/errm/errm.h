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

/* Functions that call functions which set errno on failure should have err_int
 * as a return type.
 */
#ifndef _ERROR_H_
#define _ERROR_H_

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#ifndef DURING_SIZE
#define DURING_SIZE 48
#endif

#ifndef PROBLEM_SIZE
#define PROBLEM_SIZE 208
#endif

#ifndef ENTRIES_SIZE
#define ENTRIES_SIZE 16
#endif

#define STACK_SIZE 12
#define MAX_DYN_DEFERS 12
#define MAX_STATIC_DEFERS 6
#define POOL_SIZE MAX_STATIC_DEFERS * sizeof(void *)

#define Errored errm_error_code()

enum error_codes {
    NO_PERMISSIONS = 1,
    NOT_ENOUGH_MEMORY,
    NOT_THE_EXPECTED,
    DOES_NOT_EXIST,
    LIMIT_EXCEEDED,
    PARSING_ERROR,
    DYN_SHARED_OBJ_ERROR,
    UNKNOWN_ERROR,

    ERROR_CODES_LEN,
};

void errm_error_from_errno();

void errm_error(enum error_codes err_code);

void errm_clear();

int errm_error_code();

#define on_error_return(A)                                                     \
    if (Errored)                                                               \
        return A;

typedef void (*DeferHandle)(void *);

void static_defer(void (*handle)(void *), size_t s, void *arg);
void defer(void (*handle)(void *), void *arg);
int defer_push();
int defer_pop();
void run_defers();

#define edf_func1(ret, name, type, arg)                                        \
    static inline ret errm##name(type arg);                                    \
    ret name(type arg) {                                                       \
        errm_clear();                                                          \
        int dpret = defer_push();                                              \
        if (!dpret) {                                                          \
            fprintf(stderr, "Defer-stack limit reached\n");                    \
            fprintf(stderr, "-> in function \"%s\" on %s:%d\n", #name,         \
                    __FILE__, __LINE__);                                       \
            abort();                                                           \
        }                                                                      \
        ret r = errm##name(arg);                                               \
        run_defers();                                                          \
        dpret = defer_pop();                                                   \
        if (!dpret) {                                                          \
            fprintf(stderr, "Defer-stack underflow!!\n");                      \
            abort();                                                           \
        }                                                                      \
        return r;                                                              \
    }                                                                          \
    static inline ret errm##name(type arg)

#define edf_func2(ret, name, type, arg, type1, arg1)                           \
    static inline ret errm##name(type arg, type1 arg1);                        \
    ret name(type arg, type1 arg1) {                                           \
        errm_clear();                                                          \
        int dpret = defer_push();                                              \
        if (!dpret) {                                                          \
            fprintf(stderr, "Defer-stack limit reached\n");                    \
            fprintf(stderr, "-> in function \"%s\" on %s:%d\n", #name,         \
                    __FILE__, __LINE__);                                       \
            abort();                                                           \
        }                                                                      \
        ret r = errm##name(arg, arg1);                                         \
        run_defers();                                                          \
        dpret = defer_pop();                                                   \
        if (!dpret) {                                                          \
            fprintf(stderr, "Defer-stack underflow!!\n");                      \
            abort();                                                           \
        }                                                                      \
        return r;                                                              \
    }                                                                          \
    static inline ret errm##name(type arg, type1 arg1)

#endif