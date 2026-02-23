# Единый Makefile для лабораторной работы №4
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE
PTHREAD_CFLAGS = -Wall -Wextra -std=c99 -pthread -D_GNU_SOURCE

# Исходные файлы
TASK1_3_TARGETS = parallel_min_max process_memory zombie_demo
TASK5_TARGET = psum

# Основная цель по умолчанию
all: $(TASK1_3_TARGETS) $(TASK5_TARGET)

# Задание 1: parallel_min_max с таймаутом
parallel_min_max: parallel_min_max.c
	$(CC) $(CFLAGS) -o $@ $<

# Задание 3: process_memory
process_memory: process_memory.c
	$(CC) $(CFLAGS) -o $@ $<

# Задание 2: zombie_demo
zombie_demo: zombie_demo.c
	$(CC) $(CFLAGS) -o $@ $<

# Задание 5: parallel_sum
$(TASK5_TARGET): parallel_sum.o sum_lib.o
	$(CC) $(PTHREAD_CFLAGS) -o $@ $^

parallel_sum.o: parallel_sum.c sum_lib.h
	$(CC) $(PTHREAD_CFLAGS) -c $< -o $@

sum_lib.o: sum_lib.c sum_lib.h
	$(CC) $(CFLAGS) -c $< -o $@

# Цели для отдельных заданий
task1_3: $(TASK1_3_TARGETS)

task5: $(TASK5_TARGET)

# Очистка
clean:
	rm -f $(TASK1_3_TARGETS) $(TASK5_TARGET) *.o

# Очень чистая очистка
distclean: clean
	rm -f *~

# Показать список доступных целей
help:
	@echo "Доступные цели:"
	@echo "  make all        - собрать все программы"
	@echo "  make task1_3    - собрать программы заданий 1,2,3"
	@echo "  make task5      - собрать программу задания 5"
	@echo "  make clean      - удалить скомпилированные файлы"
	@echo "  make help       - показать эту справку"

.PHONY: all clean distclean help task1_3 task5