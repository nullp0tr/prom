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
#include "shell.h"

#define SHELL "bash"

static inline size_t minimum(size_t a, size_t b) {
    if (a > b) {
        return a;
    }
    return b;
}

struct global_state globe = {
    .vterm = NULL,
    .vtscr = NULL,
    .got_sigwinch = false,
};

static inline void handle_arrows() {
    /* This is an ugly hack that deals with arrow keys not working that well
     * when running programs like vim inside the shell.
     */
    int chars[3], chars_len = 1;
    chars[0] = 033;

    chars[chars_len++] = getch();
    if (chars[chars_len - 1] == '[') {
        chars[chars_len++] = getch();
        switch (chars[chars_len - 1]) {
        case 'A':
            vterm_keyboard_key(globe.vterm, VTERM_KEY_UP, VTERM_MOD_NONE);
            return;
        case 'B':
            vterm_keyboard_key(globe.vterm, VTERM_KEY_DOWN, VTERM_MOD_NONE);
            return;
        case 'C':
            vterm_keyboard_key(globe.vterm, VTERM_KEY_RIGHT, VTERM_MOD_NONE);
            return;
        case 'D':
            vterm_keyboard_key(globe.vterm, VTERM_KEY_LEFT, VTERM_MOD_NONE);
            return;
        default:
            break;
        }
    }

    for (int i = 0; i < chars_len; i++)
        vterm_keyboard_unichar(globe.vterm, (uint32_t)chars[i], VTERM_MOD_NONE);
}

static inline int handle_input() {
    size_t buflen;
    char buf[8192];

    while (vterm_output_get_buffer_remaining(globe.vterm) > 0) {
        int ch = getch();
        if (ch == KEY_RESIZE) {
            while (ch == KEY_RESIZE) {
                ch = getch();
            }
        }

        if (ch == -1)
            break;

        if (ch == '\r') {
            // pass_to_cmd();
        }

        if (ch == 033) {
            handle_arrows();
        } else {
            vterm_keyboard_unichar(globe.vterm, (uint32_t)ch, VTERM_MOD_NONE);
        }
    }

    while ((buflen = vterm_output_get_buffer_current(globe.vterm)) > 0) {
        buflen = minimum(buflen, sizeof(buf));
        buflen = vterm_output_read(globe.vterm, buf, buflen);

        write(globe.fd_master, buf, buflen);
    }
    return 0;
}

static inline int handle_output() {
    char buf[8192];

    ssize_t bytes_read = read(globe.fd_master, buf, sizeof(buf));

    if (bytes_read <= 0) {
        if (EAGAIN == errno)
            return 0;
        return -1;
    }

    vterm_input_write(globe.vterm, buf, bytes_read);

    if (buf[0] == '\r') {
        // char test_buf[] = "[prom:error]: just a test\n\r";
        // vterm_input_write(globe.vterm, test_buf, sizeof(test_buf));
    }
    return 0;
}

static inline int slave_process(const char *shell) {
    tracee_init();

    ioctl(globe.fd_slave, TIOCSWINSZ, &globe.wsize);

    close(globe.fd_master);

    dup2(globe.fd_slave, STDIN_FILENO);
    dup2(globe.fd_slave, STDOUT_FILENO);
    dup2(globe.fd_slave, STDERR_FILENO);
    close(globe.fd_slave);

    if (setsid() == -1) {
        return -1;
    }
    if (ioctl(STDIN_FILENO, TIOCSCTTY, 1) == -1) {
        return -1;
    }

    execlp(shell, shell, (char *)NULL);
    return -1;
}

static int set_status_bar(char status[static 1], size_t len)
    __attribute__((nonnull(1)));
__attribute__((nonnull)) static int set_status_bar(char status[static 1],
                                                   size_t len) {
    memset(status, 0, len);
    int s_len = snprintf(status, len, "%s", "[prom]: prom is running...");
    if (s_len < 0) {
        return -1;
    }
    for (; (size_t)s_len < len; status[s_len++] = ' ')
        ;
    status[len - 1] = 0;
    return 0;
}

static void draw_status_bar(char status[]) {
    int y, x;
    getyx(stdscr, y, x);
    move(getmaxy(stdscr) - 1, 0);
    init_pair(1, COLOR_WHITE, COLOR_MAGENTA);
    attron(COLOR_PAIR(1));
    addstr(status);
    attroff(COLOR_PAIR(1));
    move(y, x);
}

int file_read_cb(const char path[static 1]) {
    /*
     * This callback is called when a file WRITE gets intercepted.
     */
    oflogf(INFO, "prom_callbacks.log", "CB::FILE_READ::%s\n", path);
    return 0;
}

int file_write_cb(const char path[static 1]) {
    /*
     * This callback is called when a file WRITE gets intercepted.
     */
    oflogf(INFO, "prom_callbacks.log", "CB::FILE_WRITE::%s\n", path);

    if (!strcmp(path, "trace_test.file")) {
        /* we'll just deny that file for fun for now */
        return -1;
    }
    return 0;
}

static int master_process() {
    close(globe.fd_slave);

    struct pollfd pfds[2];

    struct pollfd *ppfd_master = &pfds[0];
    ppfd_master->fd = globe.fd_master;
    ppfd_master->events = POLLIN;

    struct pollfd *ppfd_stdin = &pfds[1];
    ppfd_stdin->fd = STDIN_FILENO;
    ppfd_stdin->events = POLLIN;

    struct tracer tracer;
    struct tracer_callbacks tracer_cbs = {NULL};
    tracer_cbs.file_read = &file_read_cb;
    tracer_cbs.file_write = &file_write_cb;

    tracer_init(&tracer, &tracer_cbs, globe.pid_child);
    for (;;) {
        tracer_loop(&tracer);

        if (globe.got_sigwinch) {
            window_resize(globe.vterm, globe.fd_master, &globe.wsize);
            globe.got_sigwinch = false;
        }

        int maxx = getmaxx(stdscr) + 1;
        if (maxx > 0) {
            char *status_bar = malloc(maxx);
            // if (status_bar) {
            set_status_bar(status_bar, maxx);
            draw_status_bar(status_bar);
            free(status_bar);
            // }
        }

        int ret = poll(pfds, sizeof(pfds) / sizeof(struct pollfd), 0);
        if (ret < 0) {
            if (EINTR == errno) {
                continue;
            }
            return -1;
        } else if (ret == 0) {
            continue;
        }

        if (ppfd_master->revents & POLLIN) {
            if (handle_output() < 0) {
                break;
            }
        }

        if (ppfd_master->revents & POLLHUP) {
            break;
        }

        if (ppfd_stdin->revents & POLLIN) {
            if (handle_input() < 0) {
                break;
            }
        }
        refresh();
    }
    return 0;
}

void cleanup() {
    if (globe.vterm != NULL) {
        vterm_free(globe.vterm);
    }
    endwin();
}

void sigwinch_handler(int signo) {
    (void)signo;
    globe.got_sigwinch = true;
}

int shell_main_entry(int argc, char *argv[]) {
    (void)argc, (void)argv;
#define errord(f, ...)                                                         \
    do {                                                                       \
        logf(ERROR, stderr, f, __VA_ARGS__);                                   \
        exit(EXIT_FAILURE);                                                    \
    } while (0)
#define errord_clean(f, ...)                                                   \
    do {                                                                       \
        cleanup();                                                             \
        errord(f, __VA_ARGS__);                                                \
    } while (0)

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = &sigwinch_handler;
    if (sigaction(SIGWINCH, &sa, NULL) == -1) {
        errord("%s.\n", "could not set up signal handler");
    }

    setlocale(LC_ALL, "");

    init_ncurses();

    maxyx(&globe.wsize.ws_row, &globe.wsize.ws_col);
    globe.wsize.ws_row -= 1;
    globe.vterm = init_vterm(globe.wsize.ws_row, globe.wsize.ws_col, &globe);

    globe.fd_master = new_master_pty();
    if (globe.fd_master == -1) {
        errord_clean("new_master_pty(): %s.\n", strerror(errno));
    }

    globe.fd_slave = open(ptsname(globe.fd_master), O_RDWR);
    if (globe.fd_slave == -1) {
        errord_clean("open(): %s.\n", strerror(errno));
    }

    globe.pid_child = fork();
    if (globe.pid_child == -1) {
        errord_clean("fork(): %s.\n", strerror(errno));
    }

    else if (globe.pid_child == 0) {
        /*Do not clean up in child process.*/
        int ret = slave_process(SHELL);
        if (ret < 0) {
            errord("%s\n", strerror(errno));
        }
        return 0; // Silence warning about control reaching end of non void..
    }

    else {
        int fdm_stat = fcntl(globe.fd_master, F_GETFL);
        if (fdm_stat == -1) {
            errord_clean("fcntl(F_GETFL): %s.\n", strerror(errno));
        }
        if (fcntl(globe.fd_master, F_SETFL, fdm_stat | O_NONBLOCK) == -1) {
            errord_clean("fcntl(F_SETFL): %s.\n", strerror(errno));
        }
        int ret = master_process();
        if (ret < 0) {
            errord_clean("%s.\n", "master process had a failure");
        }
        cleanup();
        return 0;
    }
}
