
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  // fork/wait
#include <unistd.h>     // fork/sleep
#include <sys/wait.h>   // fork/wait
#include <err.h>
#include <errno.h>

int main(int argc, char** argv) {
    pid_t pid = 0;
    pid_t wait_pid = 0;
    int status = 0;
    int exit_code = EXIT_SUCCESS;

    pid = fork();
    if (pid == -1) {
        perror(NULL);
        exit_code = EXIT_FAILURE;
    } else if (pid == 0) {
        sleep(5);
        pid = getpid();
        printf("child(%d)\n", pid);
        fflush(stdout);
    } else {
        printf("parent , child pid : %d\n", pid);
        fflush(stdout);
        wait_pid = wait(&status);
        if(wait_pid == -1){
            perror(NULL);
            exit_code = EXIT_FAILURE;
        } else {
            printf ("child = %d, status=%d\n", wait_pid, status);
        }
    }
    return exit_code;
}
