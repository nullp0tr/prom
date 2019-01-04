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

#include "tracer.h"

int tracee_init() { return raise(SIGSTOP); }

int tracer_init(struct tracer *tracer, struct tracer_callbacks *cbs,
                pid_t pid_child) {
    tracer->_cbs = cbs;

    int child_status;
    waitpid(pid_child, &child_status, WSTOPPED);

    int ptrace_options = PTRACE_O_TRACEFORK | PTRACE_O_TRACESYSGOOD |
                         PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXIT;

    if (ptrace(PTRACE_SEIZE, pid_child, NULL, ptrace_options) == -1)
        return -1;
    if (ptrace(PTRACE_INTERRUPT, pid_child, NULL, NULL) == -1)
        return -1;
    if (ptrace(PTRACE_SYSCALL, pid_child, 0, NULL) == -1)
        return -1;
    if (kill(pid_child, SIGCONT) == -1)
        return -1;

    return 0;
}

static int handle_stopped(struct tracer *tracer, pid_t pid_curr, int event,
                          int sig);
int tracer_loop(struct tracer *tracer) {
    int child_status;

    pid_t pid_curr = waitpid(-1, &child_status, WNOHANG);

    if (pid_curr == 0) {
        return 0;
    }

    if (pid_curr == -1) {
        /* Error Handling needs to be added */
        return -1;
    }

    if (WIFSTOPPED(child_status)) {
        int event = child_status >> 8;
        int sig = WSTOPSIG(child_status);
        return handle_stopped(tracer, pid_curr, event, sig);
    }

    return 0;
}

static int handle_syscalls(struct tracer *tracer, pid_t pid_curr);
static int handle_sigtrap(struct tracer *tracer, pid_t pid_curr, int event);
static int handle_stopped(struct tracer *tracer, pid_t pid_curr, int event,
                          int sig) {
    switch (sig) {
    case SIGTRAP:
        return handle_sigtrap(tracer, pid_curr, event);
    case SIGTRAP | 0x80:
        return handle_syscalls(tracer, pid_curr);
    case SIGCONT:
        return ptrace(PTRACE_SYSCALL, pid_curr, NULL, NULL);
    default:
        return ptrace(PTRACE_SYSCALL, pid_curr, NULL, sig);
    }
}

/*
 * This function is supposed to call the specified callbacks when a case
 * matches, currently all it does it is resume execution.
 */
static int handle_sigtrap(struct tracer *tracer, pid_t pid_curr, int event) {
    (void)tracer;
    switch (event) {
    case (SIGTRAP | (PTRACE_EVENT_FORK << 8)):  /* fall-thru */
    case (SIGTRAP | (PTRACE_EVENT_CLONE << 8)): /* fall-thru */
    case (SIGTRAP | (PTRACE_EVENT_STOP << 8)):  /* fall-thru */
    case (SIGTRAP | (PTRACE_EVENT_EXIT << 8)):  /* fall-thru */
    default:
        return ptrace(PTRACE_SYSCALL, pid_curr, NULL, NULL);
    }
}

static int syscall_openat(struct tracer *tracer, pid_t pid_curr,
                          struct user_regs_struct *regs);
static int handle_syscalls(struct tracer *tracer, pid_t pid_curr) {
    static long last_syscall = -1;
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, pid_curr, NULL, &regs) == -1) {
        if (ESRCH == errno) {
            /* Specified proccess doesn't exist or is not currenlty being
             * traced. So just return.
             */
            return -1;
        }

        /* If there was another error try to continue the process */
        goto next;
    }

    long syscall = regs.orig_rax;

    /*We get a syscall event once on entry and once on exit.*/
    last_syscall = last_syscall == syscall ? -1 : syscall;
    int perm = 0; // Allow by default. Deny permission only if cb says so.

    if (tracer->_cbs->syscall && last_syscall != -1) {
        (*tracer->_cbs->syscall)(&regs);
    }

    if (syscall == SYS_openat && last_syscall != -1) {
        perm = syscall_openat(tracer, pid_curr, &regs);
    }

    if (perm == -1) {
        regs.orig_rax = -EPERM;
        regs.rax = -EPERM;
    }

    if (ptrace(PTRACE_SETREGS, pid_curr, NULL, &regs) == -1) {
        /* TODO: Figure out what might cause this. In the mean time abort and
         * return -1
         */
        return -1;
    }

next:
    return ptrace(PTRACE_SYSCALL, pid_curr, NULL, NULL);
}

static char *read_string(pid_t child, unsigned long addr);
static int syscall_openat(struct tracer *tracer, pid_t pid_curr,
                          struct user_regs_struct *regs) {
    bool deny_perm = false;
    char *path = read_string(pid_curr, regs->rsi);
    if (path == NULL) {
        return 0;
    }
    int path_mode = (int)regs->rdi;
    (void)path_mode; // Curruntly unused, but should be used to retrieve
                     // absolute path.
    int flags = (int)regs->rdx;

    int acc_flag = flags & O_ACCMODE;
    if (tracer->_cbs->file_read &&
        (acc_flag == O_RDONLY || acc_flag == O_RDWR)) {
        int ret = (*tracer->_cbs->file_read)(path);
        if (ret == -1) {
            deny_perm = true;
        }
    }
    if (tracer->_cbs->file_write &&
        (acc_flag == O_WRONLY || acc_flag == O_RDWR)) {
        int ret = (*tracer->_cbs->file_write)(path);
        if (ret == -1) {
            deny_perm = true;
        }
    }

    free(path);
    return deny_perm ? -1 : 0;
}

static char *read_string(pid_t child, unsigned long addr) {
    size_t allocated = 4096;
    char *val = malloc(allocated);
    if (val == NULL) {
        return val;
    }

    size_t read = 0;
    unsigned long tmp = 0;
    while (1) {
        if (read + sizeof(tmp) > allocated) {
            allocated *= 2;
            char *temp_ptr = realloc(val, allocated);
            if (temp_ptr == NULL) {
                free(val);
                return NULL;
            }
            val = temp_ptr;
        }
        errno = 0;
        tmp = ptrace(PTRACE_PEEKTEXT, child, addr + read, 0);
        if (errno != 0) {
            val[read] = 0;
            break;
        }
        memcpy(val + read, &tmp, sizeof tmp);
        if (memchr(&tmp, 0, sizeof tmp) != NULL)
            break;
        read += sizeof tmp;
    }
    return val;
}
