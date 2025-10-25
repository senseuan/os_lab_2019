CC=gcc
CFLAGS=-I. -Wall -Wextra -pthread
TARGETS=parallel_min_max parallel_sum process_memory

all: $(TARGETS)

parallel_min_max: utils.o find_min_max.o parallel_min_max.c
	$(CC) $(CFLAGS) -o parallel_min_max parallel_min_max.c utils.o find_min_max.o

parallel_sum: utils.o sum.o parallel_sum.c
	$(CC) $(CFLAGS) -o parallel_sum parallel_sum.c utils.o sum.o

process_memory: process_memory.c
	$(CC) $(CFLAGS) -o process_memory process_memory.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

find_min_max.o: find_min_max.c find_min_max.h
	$(CC) $(CFLAGS) -c find_min_max.c

sum.o: sum.c sum.h
	$(CC) $(CFLAGS) -c sum.c

clean:
	rm -f *.o $(TARGETS)

.PHONY: all clean