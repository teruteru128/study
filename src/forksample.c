
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    pid_t pid = fork();
    if (pid > 0)
    {
        waitpid(pid, NULL, 0);
    }
    return 0;
}
