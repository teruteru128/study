
#include <errno.h>
#include <fcntl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/provider.h>
#endif

#define errchk(v, f)                                                          \
    if (!v)                                                                   \
    {                                                                         \
        unsigned long err = ERR_get_error();                                  \
        fprintf(stderr, #f " : %s\n", ERR_error_string(err, NULL));           \
        return EXIT_FAILURE;                                                  \
    }

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        return 1;
    }
    FILE *f1 = fopen(argv[1], "rb");
    if (f1 == NULL)
    {
        perror("fopen f1");
        return 1;
    }
    unsigned char keybuf1[BUFSIZ];
    unsigned char keybuf2[BUFSIZ];
    struct stat st;
    if (fstat(fileno(f1), &st) != 0)
    {
        perror("fstat");
        fclose(f1);
        return 1;
    }
    size_t keyfilesize = st.st_size;
    const unsigned char prefix = 0x04;

    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
    errchk(def, OSSL_PROVIDER_load);
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    errchk(legacy, OSSL_PROVIDER_load);
    const EVP_MD *sha512 = EVP_sha512();
    errchk(sha512, EVP_sha512);
    const EVP_MD *ripemd160 = EVP_ripemd160();
    errchk(ripemd160, EVP_ripemd160);
    EVP_MD_CTX *sha512ctx1 = EVP_MD_CTX_new();
    errchk(sha512ctx1, EVP_MD_CTX_new);
    EVP_MD_CTX *sha512ctx2 = EVP_MD_CTX_new();
    errchk(sha512ctx2, EVP_MD_CTX_new);
    EVP_MD_CTX *ripemd160ctx = EVP_MD_CTX_new();
    errchk(ripemd160ctx, EVP_MD_CTX_new);
    unsigned char hash[EVP_MAX_MD_SIZE];
    errchk(EVP_DigestInit_ex2(sha512ctx1, sha512, NULL), EVP_DigestInit_ex2);
    errchk(EVP_DigestInit_ex2(sha512ctx2, sha512, NULL), EVP_DigestInit_ex2);
    for (size_t u = 0; u < keyfilesize; u += BUFSIZ)
    {
        fseek(f1, u, SEEK_SET);
        if (fread(keybuf1, 64, BUFSIZ / 64, f1) < 128)
        {
            fprintf(stderr, "fread keybuf1: %s\n", strerror(errno));
            fclose(f1);
            break;
        }
        for (size_t v = 0; v < keyfilesize; v += BUFSIZ)
        {
            fseek(f1, v, SEEK_SET);
            if (fread(keybuf2, 64, BUFSIZ / 64, f1) < 128)
            {
                fprintf(stderr, "fread keybuf2: %s\n", strerror(errno));
                fclose(f1);
                break;
            }
            for (size_t i = 0; i < BUFSIZ; i += 64)
            {
                errchk(EVP_DigestInit_ex2(sha512ctx1, sha512, NULL),
                       EVP_DigestInit_ex2);
                errchk(EVP_DigestUpdate(sha512ctx1, &prefix, 1),
                       EVP_DigestUpdate);
                errchk(EVP_DigestUpdate(sha512ctx1, keybuf1 + i, 64),
                       EVP_DigestUpdate);
                for (size_t j = 0; j < BUFSIZ; j += 64)
                {
                    errchk(EVP_MD_CTX_copy_ex(sha512ctx2, sha512ctx1),
                           EVP_MD_CTX_copy_ex);
                    errchk(EVP_DigestUpdate(sha512ctx1, &prefix, 1),
                           EVP_DigestUpdate);
                    errchk(EVP_DigestUpdate(sha512ctx1, keybuf2 + j, 64),
                           EVP_DigestUpdate);
                    unsigned int s = 0;
                    errchk(EVP_DigestFinal_ex(sha512ctx2, hash, &s),
                           EVP_DigestFinal_ex);
                    errchk(EVP_DigestInit_ex2(ripemd160ctx, ripemd160, NULL),
                           EVP_DigestInit_ex2);
                    errchk(EVP_DigestUpdate(ripemd160ctx, hash, s),
                           EVP_DigestUpdate);
                    errchk(EVP_DigestFinal_ex(ripemd160ctx, hash, &s),
                           EVP_DigestFinal_ex);
                    if ((*(unsigned long *)hash) & 0x0000ffffffffffffUL)
                    {
                        continue;
                    }
                    printf("%d, 04",
                           __builtin_clzl(be64toh(*(unsigned long *)hash)));
                    for (size_t k = 0; k < 64; k++)
                    {
                        printf("%02x", keybuf1[i + k]);
                    }
                    fputs("04", stdout);
                    for (size_t k = 0; k < 64; k++)
                    {
                        printf("%02x", keybuf2[j + k]);
                    }
                    fputs("\n", stdout);
                }
            }
        }
    }
    EVP_MD_CTX_free(sha512ctx1);
    EVP_MD_CTX_free(sha512ctx2);
    EVP_MD_CTX_free(ripemd160ctx);
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);

    return 0;
}
