
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>

UUID_DEFINE(NAMESPACE_DNS, 0x6b, 0xa7, 0xb8, 0x10, 0x9d, 0xad, 0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8);
UUID_DEFINE(NAMESPACE_URL, 0x6b, 0xa7, 0xb8, 0x11, 0x9d, 0xad, 0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8);
UUID_DEFINE(NAMESPACE_OID, 0x6b, 0xa7, 0xb8, 0x12, 0x9d, 0xad, 0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8);
UUID_DEFINE(NAMESPACE_X500, 0x6b, 0xa7, 0xb8, 0x14, 0x9d, 0xad, 0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8);

#define PYTHON_ORG "python.org"

/**
 * UUID test
 * */
int main(int argc, char **argv)
{
    uuid_t uuid;
    char out[UUID_STR_LEN];

    uuid_generate_random(uuid);
    uuid_unparse(uuid, out);
    printf("%s\n", out);
    uuid_generate_md5(uuid, NAMESPACE_DNS, PYTHON_ORG, strlen(PYTHON_ORG));
    uuid_unparse(uuid, out);
    printf("%s\n", out);
    uuid_generate_sha1(uuid, NAMESPACE_DNS, PYTHON_ORG, strlen(PYTHON_ORG));
    uuid_unparse(uuid, out);
    printf("%s\n", out);

    return EXIT_SUCCESS;
}
