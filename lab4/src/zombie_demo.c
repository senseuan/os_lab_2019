#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    
    if (pid == 0) {
        //Дочерний процесс
        printf("Child process (PID: %d) is running\n", getpid());
        printf("Child process will exit, becoming a zombie\n");
        exit(0);
    } else if (pid > 0) {
        // Родительский процесс
        printf("Parent process (PID: %d)\n", getpid());
        printf("Child process (PID: %d) has exited, but parent hasn't called wait()\n", pid);
        printf("Check zombie process with: ps aux | grep Z\n");
        printf("Press Enter to call wait() and clean up the zombie...\n");
        getchar();
        
        //Очистка зомби-процесса
        wait(NULL);
        printf("Zombie process cleaned up\n");
    } else {
        perror("fork");
        return 1;
    }
    
    return 0;
}