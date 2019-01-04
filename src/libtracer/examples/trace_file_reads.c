#include "../tracer.h"
#include <stdio.h>
#include <unistd.h>

int read_cb(const char *path) {
  printf("READ_CB::%s\n", path);
  return 0;
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "usage: %s program.\n", argv[0]);
    return 1;
  }

  pid_t pid_child = fork();
  if (pid_child == -1) {
    perror("fork()");
    return 1;
  }

  if (pid_child == 0) {
    tracee_init();
    execlp(argv[1], argv[1], (char *)NULL);
    perror("execl()");
    return -1;
  }

  else {
    struct tracer tracer;
    struct tracer_callbacks tracer_cbs = {NULL};
    tracer_cbs.file_read = &read_cb;
    tracer_init(&tracer, &tracer_cbs, pid_child);
    for (;;) {
      if (tracer_loop(&tracer) == -1)
        break;
    }
    return 0;
  }
}
