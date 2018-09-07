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