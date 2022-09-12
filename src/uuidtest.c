
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>

UUID_DEFINE(NAMESPACE_DNS, 0x6b, 0xa7, 0xb8, 0x10, 0x9d, 0xad, 0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8);
UUID_DEFINE(NAMESPACE_URL, 0x6b, 0xa7, 0xb8, 0x11, 0x9d, 0xad, 0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8);
UUID_DEFINE(NAMESPACE_OID, 0x6b, 0xa7, 0xb8, 0x12, 0x9d, 0xad, 0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8);
UUID_DEFINE(NAMESPACE_X500, 0x6b, 0xa7, 0xb8, 0x14, 0x9d, 0xad, 0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8);
UUID_DEFINE(RUNTIME_GENERATED_PROFILE_NAMESPACE_GUID, 0xf6, 0x5d, 0xdb, 0x7e, 0x70, 0x6b, 0x44, 0x99, 0x8a, 0x50, 0x40, 0x31, 0x3c, 0xaf, 0x51, 0x0a);

#define PYTHON_ORG "python.org"
#define CMD_EXE "cmd.exe"
#define CMD "Command Prompt"

/**
 * UUID test
 * */
int main(int argc, char **argv)
{
    uuid_t uuid;
    char out[UUID_STR_LEN];

    uuid_generate_random(uuid);
    uuid_unparse(uuid, out);
    printf("random: %s\n", out);
    uuid_generate_md5(uuid, NAMESPACE_DNS, PYTHON_ORG, strlen(PYTHON_ORG));
    uuid_unparse(uuid, out);
    printf("md5 with dns: %s\n", out);
    uuid_generate_sha1(uuid, NAMESPACE_DNS, PYTHON_ORG, strlen(PYTHON_ORG));
    uuid_unparse(uuid, out);
    printf("sha1 with dns: %s\n", out);
    uuid_generate_sha1(uuid, NAMESPACE_URL, PYTHON_ORG, strlen(PYTHON_ORG));
    uuid_unparse(uuid, out);
    printf("sha1 with url: %s\n", out);
    uuid_generate_sha1(uuid, NAMESPACE_OID, PYTHON_ORG, strlen(PYTHON_ORG));
    uuid_unparse(uuid, out);
    printf("sha1 with oid: %s\n", out);
    uuid_generate_sha1(uuid, NAMESPACE_X500, PYTHON_ORG, strlen(PYTHON_ORG));
    uuid_unparse(uuid, out);
    printf("sha1 with x500: %s\n", out);
    uuid_generate_sha1(uuid, RUNTIME_GENERATED_PROFILE_NAMESPACE_GUID, CMD, strlen(CMD));
    uuid_unparse(uuid, out);
    printf("sha1 with guid: %s\n", out);

    return EXIT_SUCCESS;
}
