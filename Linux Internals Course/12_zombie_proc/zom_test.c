#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    pid_t child_pid;

    child_pid = fork(); // Create a child process

    if (child_pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (child_pid == 0) {
        printf("Child process executing: %d\n", getpid());
        exit(0);
    } else {
        // The parent process does not handle the termination of the child process
        // It does not invoke wait() or waitpid() to collect the exit status

        printf("Parent process executing: %d\n", getpid());
        printf("Zombie process created\n");

        sleep(200); 
    }

    return 0;
}

