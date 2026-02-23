// parallel_min_max.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <time.h>

// Добавляем недостающие заголовки
#include <sys/types.h>

// Функция генерации массива из лабораторной работы №3
int* generate_array(int size, int seed) {
    srand(seed);
    int* array = malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 1000;
    }
    return array;
}

// Функция поиска минимума в диапазоне
int find_min(int* array, int begin, int end) {
    int min = array[begin];
    for (int i = begin + 1; i < end; i++) {
        if (array[i] < min) {
            min = array[i];
        }
    }
    return min;
}

// Функция поиска максимума в диапазоне
int find_max(int* array, int begin, int end) {
    int max = array[begin];
    for (int i = begin + 1; i < end; i++) {
        if (array[i] > max) {
            max = array[i];
        }
    }
    return max;
}

// Обработчик сигнала таймаута
void alarm_handler(int sig) {
    (void)sig; // Подавляем предупреждение о неиспользуемом параметре
    printf("\nTimeout reached! Killing all child processes...\n");
    // Родительский процесс отправит SIGKILL дочерним в основном коде
}

int main(int argc, char** argv) {
    int threads_num = 0;
    int array_size = 0;
    int seed = 0;
    int timeout = 0;
    int timeout_set = 0;
    
    // Парсинг аргументов командной строки
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--threads_num") == 0 && i + 1 < argc) {
            threads_num = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--array_size") == 0 && i + 1 < argc) {
            array_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--timeout") == 0 && i + 1 < argc) {
            timeout = atoi(argv[++i]);
            timeout_set = 1;
        }
    }
    
    if (threads_num <= 0 || array_size <= 0) {
        printf("Usage: %s --threads_num N --seed S --array_size M [--timeout T]\n", argv[0]);
        return 1;
    }
    
    // Генерация массива
    int* array = generate_array(array_size, seed);
    
    // Создание pipe для коммуникации
    int pipes[threads_num][2];
    pid_t pids[threads_num];
    
    // Разделение массива между процессами
    int chunk_size = array_size / threads_num;
    
    for (int i = 0; i < threads_num; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return 1;
        }
        
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Дочерний процесс
            close(pipes[i][0]); // Закрываем чтение
            
            int begin = i * chunk_size;
            int end = (i == threads_num - 1) ? array_size : (i + 1) * chunk_size;
            
            int min_val = find_min(array, begin, end);
            int max_val = find_max(array, begin, end);
            
            // Отправляем результаты родителю
            write(pipes[i][1], &min_val, sizeof(int));
            write(pipes[i][1], &max_val, sizeof(int));
            
            close(pipes[i][1]);
            free(array);
            exit(0);
        } else if (pids[i] > 0) {
            // Родительский процесс
            close(pipes[i][1]); // Закрываем запись
        } else {
            perror("fork");
            return 1;
        }
    }
    
    // Установка обработчика сигнала таймаута
    if (timeout_set) {
        signal(SIGALRM, alarm_handler);
        alarm(timeout);
    }
    
    // Ожидание завершения дочерних процессов
    int global_min = array[0];
    int global_max = array[0];
    int completed = 0;
    
    while (completed < threads_num) {
        int status;
        pid_t result = waitpid(-1, &status, WNOHANG);
        
        if (result == -1) {
            perror("waitpid");
            break;
        } else if (result > 0) {
            // Дочерний процесс завершился
            for (int i = 0; i < threads_num; i++) {
                if (pids[i] == result) {
                    int min_val, max_val;
                    read(pipes[i][0], &min_val, sizeof(int));
                    read(pipes[i][0], &max_val, sizeof(int));
                    
                    if (min_val < global_min) global_min = min_val;
                    if (max_val > global_max) global_max = max_val;
                    
                    close(pipes[i][0]);
                    completed++;
                    break;
                }
            }
        } else {
            // Нет завершенных процессов
            if (timeout_set && alarm(0) > 0) {
                // Таймаут еще не наступил
                struct timespec ts;
                ts.tv_sec = 0;
                ts.tv_nsec = 100000000; // 0.1 секунды в наносекундах
                nanosleep(&ts, NULL);
            }
        }
    }
    
    // Если был таймаут и не все процессы завершились
    if (completed < threads_num) {
        printf("Timeout reached, killing remaining processes...\n");
        for (int i = 0; i < threads_num; i++) {
            if (pids[i] > 0) {
                kill(pids[i], SIGKILL);
            }
        }
        printf("All processes killed.\n");
    } else {
        printf("Global min: %d\n", global_min);
        printf("Global max: %d\n", global_max);
    }
    
    free(array);
    return 0;
}