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
#include "pty.h"

int new_master_pty() {
    int fd_master;
    fd_master = posix_openpt(O_RDWR);
    if (fd_master < 0 || grantpt(fd_master) != 0 || unlockpt(fd_master) != 0) {
        return -1;
    }
    return fd_master;
}

int obtain_slave_pty(int fd_master) { return open(ptsname(fd_master), O_RDWR); }
