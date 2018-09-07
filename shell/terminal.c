#include "terminal.h"

void maxyx(unsigned short *rows, unsigned short *cols) {
    *rows = getmaxy(stdscr);
    *cols = getmaxx(stdscr);
}

int print_cell(VTermScreenCell *cell, int row, int col) {
    attr_t attr = A_NORMAL;
    short pair = 0;
    cchar_t cch;
    const wchar_t *wch;

    wch = cell->chars[0] ? (wchar_t *)&cell->chars[0] : L" ";

    if (cell->attrs.blink)
        attr |= A_BLINK;
    if (cell->attrs.bold)
        attr |= A_BOLD;
    if (cell->attrs.italic)
        attr |= A_ITALIC;
    if (cell->attrs.reverse)
        attr |= A_REVERSE;
    if (cell->attrs.underline)
        attr |= A_UNDERLINE;

    if (setcchar(&cch, wch, attr, pair, NULL) == ERR)
        return -1;
    if (move(row, col) == ERR)
        return -1;
    if (add_wch(&cch) == ERR)
        return -1;

    return 0;
}

int damage_callback(VTermRect rect, void *user) {
    struct global_state *globe = (struct global_state *)user;

    int y, x;
    getyx(stdscr, y, x);

    for (int row = rect.start_row; row < rect.end_row; row++) {
        for (int col = rect.start_col; col < rect.end_col; col++) {
            VTermScreen *vtscreen = vterm_obtain_screen(globe->vterm);
            VTermScreenCell cell;
            vterm_screen_get_cell(vtscreen, (VTermPos){row, col}, &cell);
            print_cell(&cell, row, col);
        }
    }

    move(y, x);
    return 1;
}

int movecursor_callback(VTermPos pos, VTermPos oldpos, int visible,
                        void *user) {
    (void)user, (void)visible, (void)user, (void)oldpos;
    if (move(pos.row, pos.col) == ERR) {
        return 0;
    }
    return 1;
}

VTermScreenCallbacks vtscreen_cbs = {
    .damage = &damage_callback,
    .movecursor = &movecursor_callback,
};

void free_terminal(VTerm *vterm) { vterm_free(vterm); }

int init_ncurses() {
    if (initscr() == NULL) {
        return -1;
    }
    if (raw() == ERR) {
        return -1;
    }
    if (noecho() == ERR) {
        return -1;
    }
    if (nodelay(stdscr, true) == ERR) {
        return -1;
    }
    if (keypad(stdscr, false) == ERR) {
        return -1;
    }
    if (nonl() == ERR) {
        return -1;
    }
    if (meta(stdscr, false) == ERR) {
        return -1;
    }

    refresh();
    return 0;
}

void window_resize(VTerm *vterm, int fd_term, struct winsize *wsize) {
    vterm_screen_flush_damage(vterm_obtain_screen(vterm));
    endwin();
    refresh();
    maxyx(&wsize->ws_row, &wsize->ws_col);
    wsize->ws_row -= 1;
    vterm_set_size(vterm, wsize->ws_row, wsize->ws_col);
    ioctl(fd_term, TIOCSWINSZ, wsize);
}

void free_ncurses() { endwin(); }

VTerm *init_vterm(unsigned short int row, unsigned short int col, void *user) {
    VTerm *vterm = vterm_new(row, col);

    // vterm_state_set_default_colors(vterm_obtain_state(vterm),
    //                                (VTermColor *)&DEFAULT_COLOR,
    //                                (VTermColor *)&DEFAULT_COLOR);
    vterm_set_utf8(vterm, 1);

    VTermScreen *vtscreen = vterm_obtain_screen(vterm);
    vterm_screen_enable_altscreen(vtscreen, 1);
    vterm_screen_reset(vtscreen, 1);

    vterm_screen_set_callbacks(vtscreen, &vtscreen_cbs, user);
    return vterm;
}