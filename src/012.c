
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>

/**
 * UUID test
 * */
int main(int argc, char **argv)
{
    uuid_t uuid;
    char out[37];

    uuid_generate_random(uuid);
    uuid_unparse(uuid, out);
    printf("%s(%zd)\n", out, strlen(out));

    return 0;
}
