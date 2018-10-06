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
#define _XOPEN_SOURCE 600

#include "../libtracer/tracer.h"
#include "../libvterm/include/vterm.h"
#include "../utils/logger.h"
#include "error.h"
#include "pty.h"
#include "terminal.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <ncursesw/curses.h>
#include <poll.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/select.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <termios.h>
#include <unistd.h>

int shell_main_entry();