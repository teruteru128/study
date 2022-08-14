
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <err.h>
#include <random.h>
#include <shuffle.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>

#define MAX 1000000

int main(int argc, char **argv)
{
    getrandom(getseedp(), sizeof(uint32_t), GRND_NONBLOCK);

    char messages[] = "ファルコン・パンチ";
    size_t messages_size = 9;
    size_t i = 0;
    size_t j = 0;
    for (; j < MAX; j++)
    {
        shuffle(messages, 3, strlen(messages) / 3);
        fprintf(stdout, "%s\n", messages);
    }

    return EXIT_SUCCESS;
}
