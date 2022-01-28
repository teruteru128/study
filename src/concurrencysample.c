
#include <stddef.h>
#include <pthread.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    printf("%d\n", pthread_getconcurrency());
    return 0;
}
