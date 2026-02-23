#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    long long start;
    long long end;
    long long mod;
    long long *result;
    pthread_mutex_t *mutex;
} ThreadData;

void* compute_partial(void* arg) {
    ThreadData *data = (ThreadData*)arg;
    long long partial = 1;
    
    for (long long i = data->start; i <= data->end; i++) {
        partial = (partial * i) % data->mod;
    }
    
    pthread_mutex_lock(data->mutex);
    *(data->result) = (*(data->result) * partial) % data->mod;
    pthread_mutex_unlock(data->mutex);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    long long k = 0, mod = 0;
    int pnum = 1;
    
    // Парсинг аргументов командной строки
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            k = atoll(argv[++i]);
        } 
        else if (strncmp(argv[i], "--pnum=", 7) == 0) {
            pnum = atoi(argv[i] + 7);
        }
        else if (strncmp(argv[i], "--mod=", 6) == 0) {
            mod = atoll(argv[i] + 6);
        }
    }
    
    if (k <= 0 || mod <= 0 || pnum <= 0) {
        printf("Usage: %s -k <number> --pnum=<threads> --mod=<modulus>\n", argv[0]);
        printf("Example: %s -k 10 --pnum=4 --mod=1000\n", argv[0]);
        return 1;
    }
    
    printf("Computing %lld! mod %lld using %d threads\n", k, mod, pnum);
    
    pthread_t threads[pnum];
    ThreadData thread_data[pnum];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    long long result = 1;
    
    long long chunk_size = k / pnum;
    long long remainder = k % pnum;
    long long current = 1;
    
    // Создание потоков
    for (int i = 0; i < pnum; i++) {
        thread_data[i].start = current;
        thread_data[i].end = current + chunk_size - 1;
        
        // Распределяем остаток между первыми remainder потоками
        if (i < remainder) {
            thread_data[i].end++;
        }
        
        // Убеждаемся, что не выходим за пределы k
        if (thread_data[i].end > k) {
            thread_data[i].end = k;
        }
        
        thread_data[i].mod = mod;
        thread_data[i].result = &result;
        thread_data[i].mutex = &mutex;
        
        printf("Thread %d: computing from %lld to %lld\n", i, thread_data[i].start, thread_data[i].end);
        
        if (thread_data[i].start <= k && thread_data[i].start <= thread_data[i].end) {
            pthread_create(&threads[i], NULL, compute_partial, &thread_data[i]);
        }
        
        current = thread_data[i].end + 1;
        if (current > k) break;
    }
    
    // Ожидание завершения потоков
    for (int i = 0; i < pnum; i++) {
        if (thread_data[i].start <= k && thread_data[i].start <= thread_data[i].end) {
            pthread_join(threads[i], NULL);
        }
    }
    
    printf("%lld! mod %lld = %lld\n", k, mod, result);
    
    return 0;
}