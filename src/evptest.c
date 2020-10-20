
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

#define str "getting over it with bennett foddy"

int main(int argc, char const *argv[])
{
    OpenSSL_add_all_digests();
    EVP_MD_CTX *ctx = EVP_MD_CTX_create();

    unsigned int mdsize = 0;
    unsigned char md[EVP_MAX_MD_SIZE];

    EVP_DigestInit(ctx, EVP_get_digestbynid(NID_sha3_256));
    EVP_DigestUpdate(ctx, str, strlen(str));
    EVP_DigestFinal(ctx, md, &mdsize);
    for (unsigned int i = 0; i < mdsize; i++)
    {
        fprintf(stdout, "%02x", md[i]);
    }
    fputs("\n", stdout);

    EVP_DigestInit(ctx, EVP_sha3_256());
    EVP_DigestUpdate(ctx, str, strlen(str));
    EVP_DigestFinal(ctx, md, &mdsize);
    for (unsigned int i = 0; i < mdsize; i++)
    {
        fprintf(stdout, "%02x", md[i]);
    }
    fputs("\n", stdout);

    EVP_MD_CTX_free(ctx);
    EVP_cleanup();
    return 0;
}
