
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

#include "safe.h"

int main(int argc, char** argv) {
    EVP_MD_CTX mdctx;
    const EVP_MD *md;
    char mess1[] = "MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s=";
    char mess2[] = "10895332777036447559";
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    int i;

    OpenSSL_add_all_digests();

    if(!argv[1]) {
        printf("Usage: %s digestname\n", argv[0]);
        exit(1);
    }

    md = EVP_get_digestbyname(argv[1]);

    if(!md) {
        printf("Unknown message digest %s\n", argv[1]);
        exit(1);
    }
    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit_ex(&mdctx, md, NULL);
    EVP_DigestUpdate(&mdctx, mess1, strlen(mess1));
    EVP_DigestUpdate(&mdctx, mess2, strlen(mess2));
    EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
    EVP_MD_CTX_cleanup(&mdctx);

    printf("Digest is: ");
    for(i = 0; i < md_len; i++) printf("%02x", md_value[i]);
    printf("\n");
    return EXIT_SUCCESS;
}
