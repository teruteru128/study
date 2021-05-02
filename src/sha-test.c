
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sha-1.h>

#define DATA "The quick brown fox jumps over the lazy dog"

int main(int argc, char *argv[])
{

    SHA_CTX sha;

    sha_init(&sha);
    sha_update(&sha, DATA, strlen(DATA));

    char buf[20];
    char txt[41];
    sha_finish(buf, &sha);

    return EXIT_SUCCESS;
}
