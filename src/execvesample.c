
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char const *argv[], char *envp[])
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
    }
    else if (pid == 0)
    {
        char *const cmd[] = {"/usr/bin/sl", "-al", "--color", NULL};
        execve(cmd[0], cmd, envp);
        perror("execve");
        exit(3);
    }
    else
    {
        int status;
        int ret = wait(&status);
        if (ret < 0)
        {
            perror("wait");
            exit(4);
        }
        fprintf(stderr, "child process exit code: %d, %d\n", WEXITSTATUS(status), WIFEXITED(status));
    }
    return 0;
}
