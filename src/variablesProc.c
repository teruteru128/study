#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>

int varA;

void processFunc(int n)
{
    int varB = 0;
    varB = 4 * n;
    printf("processFunc-%d-1: varA=%d, varB=%d\n", n, varA, varB);
    varA = 5 * n;
    printf("processFunc-%d-2: varA=%d, varB=%d\n", n, varA, varB);
    sleep(2);
    printf("processFunc-%d-3: varA=%d, varB=%d\n", n, varA, varB);
    varB = 6 * n;
    printf("processFunc-%d-4: varA=%d, varB=%d\n", n, varA, varB);

    exit(0);
}

int main(void)
{
    pid_t process1, process2;
    int varB;
    varA = 1;
    varB = 2;
    printf("main-1 : varA=%d, varB=%d\n", varA, varB);
    if((process1 = fork()) == 0)
    {
        // this is child process
        processFunc(1);
    }
    // this is main process
    sleep(1);
    varB = 3;
    printf("main-2 : varA=%d, varB=%d\n", varA, varB);
    if((process2 = fork()) == 0)
    {
        // this is child process
        processFunc(2);
    }
    // this is main process
    waitpid(process1, NULL, 0);
    waitpid(process2, NULL, 0);
    printf("main-3 : varA=%d, varB=%d\n", varA, varB);

    return 0;
}
