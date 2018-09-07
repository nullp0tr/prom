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