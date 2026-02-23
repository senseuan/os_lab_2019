#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool with_files = false;

    while (true) {
        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0: // seed
                        seed = atoi(optarg);
                        if (seed <= 0) {
                            printf("seed must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 1: // array_size
                        array_size = atoi(optarg);
                        if (array_size <= 0) {
                            printf("array_size must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 2: // pnum
                        pnum = atoi(optarg);
                        if (pnum <= 0) {
                            printf("pnum must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 3: // by_files
                        with_files = true;
                        break;
                }
                break;
            case 'f':
                with_files = true;
                break;
            case '?':
                break;
            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed <num> --array_size <num> --pnum <num> [--by_files]\n", argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    if (!array) {
        printf("Memory allocation failed\n");
        return 1;
    }
    GenerateArray(array, array_size, seed);
    
    int active_child_processes = 0;
    int (*pipes)[2] = NULL;
    char filenames[pnum][20];
    
    if (!with_files) {
        pipes = malloc(pnum * sizeof(*pipes));
        if (!pipes) {
            printf("Memory allocation for pipes failed\n");
            free(array);
            return 1;
        }
        for (int i = 0; i < pnum; i++) {
            if (pipe(pipes[i]) == -1) {
                printf("Pipe creation failed!\n");
                free(pipes);
                free(array);
                return 1;
            }
        }
    }

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    // Разбиваем массив на части для каждого процесса
    int chunk_size = array_size / pnum;
    int remainder = array_size % pnum;
    int start = 0;

    for (int i = 0; i < pnum; i++) {
        int current_chunk_size = chunk_size + (i < remainder ? 1 : 0);
        int end = start + current_chunk_size;
        
        pid_t child_pid = fork();
        
        if (child_pid == -1) {
            printf("Fork failed!\n");
            free(array);
            if (!with_files && pipes) free(pipes);
            return 1;
        }
        
        if (child_pid == 0) {
            // Child process
            struct MinMax local_min_max = GetMinMax(array, start, end);
            
            if (with_files) {
                // Используем файлы
                sprintf(filenames[i], "temp_%d.txt", i);
                FILE *file = fopen(filenames[i], "w");
                if (file) {
                    fprintf(file, "%d %d", local_min_max.min, local_min_max.max);
                    fclose(file);
                }
            } else {
                // Используем pipe
                close(pipes[i][0]); // закрываем чтение
                write(pipes[i][1], &local_min_max, sizeof(struct MinMax));
                close(pipes[i][1]);
            }
            
            free(array);
            if (!with_files && pipes) free(pipes);
            return 0;
        } else {
            // Parent process
            active_child_processes++;
            if (!with_files) {
                close(pipes[i][1]); // родитель закрывает запись
            }
        }
        
        start = end;
    }

    // Parent process - ожидание завершения всех детей
    while (active_child_processes > 0) {
        wait(NULL);
        active_child_processes--;
    }

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        struct MinMax local_result;
        
        if (with_files) {
            // Читаем из файлов
            sprintf(filenames[i], "temp_%d.txt", i);
            FILE *file = fopen(filenames[i], "r");
            if (file) {
                if (fscanf(file, "%d %d", &local_result.min, &local_result.max) != 2) {
                    printf("Error reading from file %s\n", filenames[i]);
                }
                fclose(file);
                remove(filenames[i]); // удаляем временный файл
            }
        } else {
            // Читаем из pipe
            if (read(pipes[i][0], &local_result, sizeof(struct MinMax)) != sizeof(struct MinMax)) {
                printf("Error reading from pipe %d\n", i);
            }
            close(pipes[i][0]);
        }

        if (local_result.min < min_max.min) min_max.min = local_result.min;
        if (local_result.max > min_max.max) min_max.max = local_result.max;
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);
    if (!with_files && pipes) free(pipes);

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    fflush(NULL);
    
    return 0;
}