#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread1_func(void* arg) {
    printf("Поток 1: пытается захватить мьютекс 1...\n");
    pthread_mutex_lock(&mutex1);
    printf("Поток 1: захватил мьютекс 1\n");
    
    // Имитация работы
    sleep(1);
    
    printf("Поток 1: пытается захватить мьютекс 2...\n");
    pthread_mutex_lock(&mutex2);
    printf("Поток 1: захватил мьютекс 2\n");
    
    // Критическая секция
    printf("Поток 1: работаю с обоими ресурсами\n");
    
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    
    return NULL;
}

void* thread2_func(void* arg) {
    printf("Поток 2: пытается захватить мьютекс 2...\n");
    pthread_mutex_lock(&mutex2);
    printf("Поток 2: захватил мьютекс 2\n");
    
    // Имитация работы
    sleep(1);
    
    printf("Поток 2: пытается захватить мьютекс 1...\n");
    pthread_mutex_lock(&mutex1);
    printf("Поток 2: захватил мьютекс 1\n");
    
    // Критическая секция
    printf("Поток 2: работаю с обоими ресурсами\n");
    
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    
    printf("Создание потоков...\n");
    
    pthread_create(&thread1, NULL, thread1_func, NULL);
    pthread_create(&thread2, NULL, thread2_func, NULL);
    
    // Ждем завершения потоков (но они зависнут в deadlock)
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("Программа завершена\n");
    
    return 0;
}