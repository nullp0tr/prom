#define _XOPEN_SOURCE 600
#include "filesystem/filesystem.h"
#include "prom.h"
#include "shell/shell.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <wordexp.h>

/**
 *  Config:
 *  prom.shell.conf
 *  shellName: "bash"
 *  promptStringChanger: "PROMPS1='!p> '; PS1=${PS1%?}$PROMPS1\n"
 *
 */

int shell_main(int argc, char *argv[]) {
    (void)argc, (void)argv;
    return shell_main_entry(argc, argv);
}