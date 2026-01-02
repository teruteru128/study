
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char const *argv[], char *envp[])
{
    pid_t pid;
    int status;
    int ret;
    if ((pid = fork()) < 0)
    {
        perror("fork");
    }
    else if (pid == 0)
    {
        char *const cmd[] = {"/usr/bin/ls", "-al", "--color", NULL};
        execve(cmd[0], cmd, envp);
        perror("execve");
        exit(3);
    }
    else
    {
        if ((ret = wait(&status)) < 0)
        {
            perror("wait");
            exit(4);
        }
    }
    return 0;
}
