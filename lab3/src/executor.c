#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s seed arraysize\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    
    if (pid == -1) {
        printf("Fork failed!\n");
        return 1;
    } else if (pid == 0) {
        // Дочерний процесс
        char *args[] = {"./sequential_min_max", argv[1], argv[2], NULL};
        execv(args[0], args);
        
        // Если execv вернулся - ошибка
        printf("Exec failed!\n");
        return 1;
    } else {
        // Родительский процесс
        int status;
        wait(&status);
        printf("Child process finished with status %d\n", status);
    }
    
    return 0;
}