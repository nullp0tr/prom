/*  libtracer: a callback based ptracing library.
    Copyright (C) 2018  Ahmed Alsharif

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef _TRACER_H_
#define _TRACER_H_

#define _XOPEN_SOURCE 600

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

/* This struct is used for keeping track of the tracer state like its callbacks,
 * do not modify directly!
 */
struct tracer {
    struct tracer_callbacks *_cbs;
};

/* All functions in this struct except for syscall should return 0 to resume
 * normally, or return -1 to return a permission denied error to the calling
 * program. Syscall is the exception because it allows the passed function to
 * manipulate the registers directly.
 */
struct tracer_callbacks {
    int (*file_read)(const char *path);
    int (*file_write)(const char *path);
    void (*syscall)(struct user_regs_struct *regs);
};

/* This should be called from inside the forked child or thread as soon as
 * execution starts, so before any calls to execv.. or anything else is done.
 */
int tracee_init();

/* This should be called in the parent process ONLY ONCE. */
int tracer_init(struct tracer *tracer, struct tracer_callbacks *cbs,
                pid_t pid_child);

/* This should be called in the parent process in a loop, since it doesn't block
 * execution.
 */
int tracer_loop(struct tracer *tracer);

#endif
