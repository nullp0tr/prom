OBJS = ../tracer.o
CFLAGS = -Wall -Wextra
CC = gcc

trace_file_reads: trace_file_reads.c $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) trace_file_reads.c -o $@

$(OBJS): ../tracer.c
	cd .. && $(MAKE)

clean:
	rm -rf trace_file_reads
