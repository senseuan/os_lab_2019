#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        printf("fork failed");
        return 1;
    }

    if (pid == 0) {
        if (execl("./sequential_min_max", "sequential_min_max", \
        "--seed", "3", "--array_size", "50", "--pnum", "1", NULL) == -1) {
            printf("execl failed");
            return 1;
        }
    } else {
        printf("Запущен процесс sequential_min_max с PID: %d\n", pid);
        wait(NULL);
    }

    return 0;
}
