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
enum LOG_LEVELS {
    ERROR,
    WARN,
    INFO,
    DEBUG,
};

#define LOG_LVL_TO_MSG(lvl)                                                    \
    (lvl == ERROR ? "Error"                                                    \
                  : lvl == WARN ? "Warning" : lvl == INFO ? "Info" : "Debug")

#define logf(lvl, fp, f, ...)                                                  \
    do {                                                                       \
        fprintf(fp, "%s: ", LOG_LVL_TO_MSG(lvl));                              \
        fprintf(fp, f, __VA_ARGS__);                                           \
    } while (0)

#define oflogf(lvl, file, f, ...)                                              \
    do {                                                                       \
        FILE *log_fp = fopen(file, "a");                                       \
        logf(lvl, log_fp, f, __VA_ARGS__);                                     \
        fclose(log_fp);                                                        \
    } while (0)

#define oflog(lvl, file, msg)                                                  \
    do {                                                                       \
        oflogf(lvl, file, "%s.\n", msg);                                       \
    } while (0)

#define log(lvl, fp, msg)                                                      \
    do {                                                                       \
        logf(lvl, fp, "%s.\n", msg);                                           \
    } while (0)
