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
