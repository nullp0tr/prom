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

#include "../libvterm/include/vterm.h"
#include "error.h"
#include <fcntl.h>
#include <locale.h>
#include <ncursesw/curses.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

struct global_state {
    VTerm *vterm;
    VTermScreen *vtscr;
    struct winsize wsize;
    int fd_master;
    int fd_slave;
    pid_t pid_child;
    bool got_sigwinch;
};

VTermScreenCallbacks vtscreen_cbs;

void free_terminal(VTerm *vterm);

void maxyx(unsigned short *rows, unsigned short *cols);

int init_ncurses();
void free_ncurses();

int damage_callback(VTermRect rect, void *user);
int pushline_callback(int cols, const VTermScreenCell *cells, void *user);

VTerm *init_vterm(unsigned short int row, unsigned short int col, void *user);

void window_resize(VTerm *vterm, int fd_term, struct winsize *wsize);