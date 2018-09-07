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
