
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    for (size_t i = 0; i < 10; i++)
    {
        fork();
    }
    return 0;
}
