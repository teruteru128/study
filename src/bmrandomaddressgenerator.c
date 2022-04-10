
#include <bitmessage.h>
#include <bm.h>
#include <stdio.h>
#include <sys/random.h>

int main(int argc, char const *argv[])
{
    char ripe[20] = "";
    char *address = NULL;
    for (size_t i = 0; i < 16777216UL; i++)
    {
        getrandom(ripe + 1, 19, GRND_NONBLOCK);
        address = encodeV4Address(ripe, 20);
        printf("%s\n", address);
        free(address);
    }

    return 0;
}
