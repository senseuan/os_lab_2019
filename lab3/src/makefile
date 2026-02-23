CC=gcc
CFLAGS=-I.
TARGETS=sequential_min_max parallel_min_max executor

all: $(TARGETS)

sequential_min_max: sequential_min_max.c utils.o find_min_max.o
	$(CC) -o sequential_min_max sequential_min_max.c utils.o find_min_max.o $(CFLAGS)

parallel_min_max: parallel_min_max.c utils.o find_min_max.o
	$(CC) -o parallel_min_max parallel_min_max.c utils.o find_min_max.o $(CFLAGS)

executor: executor.c
	$(CC) -o executor executor.c $(CFLAGS)

utils.o: utils.c utils.h
	$(CC) -c utils.c $(CFLAGS)

find_min_max.o: find_min_max.c find_min_max.h utils.h
	$(CC) -c find_min_max.c $(CFLAGS)

clean:
	rm -f *.o $(TARGETS) temp_*.txt

.PHONY: all clean