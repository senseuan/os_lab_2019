#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include "sum_lib.h"

void *ThreadSum(void *args) {
    struct SumArgs *sum_args = (struct SumArgs *)args;
    int *result = malloc(sizeof(int));
    *result = Sum(sum_args);
    return (void*)result;
}

int main(int argc, char **argv) {
    uint32_t threads_num = 0;
    uint32_t array_size = 0;
    uint32_t seed = 0;
    
    // Парсинг аргументов командной строки
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--threads_num") == 0 && i + 1 < argc) {
            threads_num = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--array_size") == 0 && i + 1 < argc) {
            array_size = atoi(argv[++i]);
        }
    }
    
    if (threads_num <= 0 || array_size <= 0 || threads_num > array_size) {
        printf("Usage: %s --threads_num N --seed S --array_size M\n", argv[0]);
        printf("Note: threads_num must be <= array_size\n");
        return 1;
    }
    
    // Генерация массива
    int *array = generate_array(array_size, seed);
    if (!array) {
        return 1;
    }
    
    // Создание потоков и замер времени
    pthread_t threads[threads_num];
    struct SumArgs args[threads_num];
    
    // Разделение массива между потоками
    int chunk_size = array_size / threads_num;
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    for (uint32_t i = 0; i < threads_num; i++) {
        args[i].array = array;
        args[i].begin = i * chunk_size;
        args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * chunk_size;
        
        if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i]) != 0) {
            printf("Error: pthread_create failed!\n");
            free(array);
            return 1;
        }
    }
    
    int total_sum = 0;
    for (uint32_t i = 0; i < threads_num; i++) {
        int *sum;
        pthread_join(threads[i], (void **)&sum);
        total_sum += *sum;
        free(sum);
    }
    
    gettimeofday(&end, NULL);
    
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;
    
    free(array);
    printf("Total sum: %d\n", total_sum);
    printf("Time elapsed: %.6f seconds\n", elapsed);
    
    return 0;
}