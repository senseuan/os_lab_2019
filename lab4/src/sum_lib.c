#include "sum_lib.h"
#include <stdlib.h>
#include <stdio.h>

int Sum(const struct SumArgs *args) {
    int sum = 0;
    for (int i = args->begin; i < args->end; i++) {
        sum += args->array[i];
    }
    return sum;
}

int* generate_array(int size, int seed) {
    srand(seed);
    int* array = malloc(sizeof(int) * size);
    if (!array) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 1000;
    }
    return array;
}