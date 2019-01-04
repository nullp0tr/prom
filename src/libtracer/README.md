# libtracer
libtracer is callback based ptracing library. It's not in a usable state, and it only supports x86\_64 currently and probably forever. Here's an example:  


```c
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
```

# Running the examples

```bash
$ git clone this..
$ cd libtracer/examples
$ make
$ ./trace_file_reads ls

```

the last outputs all file reads that `ls` makes, for example:
```
READ_CB::/etc/ld.so.cache
READ_CB::/lib/x86_64-linux-gnu/libselinux.so.1
READ_CB::/lib/x86_64-linux-gnu/libc.so.6
READ_CB::/lib/x86_64-linux-gnu/libpcre.so.3
READ_CB::/lib/x86_64-linux-gnu/libdl.so.2
READ_CB::/lib/x86_64-linux-gnu/libpthread.so.0
READ_CB::/proc/filesystems
READ_CB::/usr/lib/locale/locale-archive
READ_CB::.
Makefile  trace_file_reads  trace_file_reads.c
```

# Supported Callbacks

Currently 3 callbacks are supported, `file_read` and `file_write` are gonna break soon to supply absolute path of file open. 

#### `tracer_callbacks.file_read`
This gets called with the path of a file that was open for reading. Currently a relative path is supplied.
#### `tracer_callbacks.file_write`
This gets called with the path of af ile that was open for writing. Currently a relative path is supplied.
#### `tracer_callbacks.syscall`
This gets called before every syscall entry with a pointer to the registers, any modifications to the registers would be propogated to the kernel.

