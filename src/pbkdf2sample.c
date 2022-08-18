
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include "yattaze.h"
#include <openssl/opensslv.h>
#include <string.h>

#include <openssl/evp.h>

#define KEYLEN 64
#define SALTLEN 64

int main(int argc, char const *argv[])
{

    unsigned char out[KEYLEN];
    unsigned char salt[SALTLEN] = "";
    if (PKCS5_PBKDF2_HMAC(YATTAZE, -1, salt, SALTLEN, 100000, EVP_sha512(), KEYLEN,
                          out))
    {
        for (size_t i = 0; i < KEYLEN; i++)
        {
            printf("%02x", out[i]);
        }
        printf("\n");
    }

    return 0;
}
