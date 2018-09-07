#include <sys/user.h>

struct module_cbs {
    int (*read_file)(const char *path);
    int (*write_file)(const char *path);
    void (*syscall)(struct user_regs_struct *regs);
    void (*cmd)(const char *cmd);
};
