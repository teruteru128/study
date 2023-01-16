
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1
#include <bm.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <stdint.h>
#include <stdio.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/provider.h>
#endif

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "%s [passphrease]\n", argv[0]);
        return 1;
    }
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
#endif
    const int numberOfNullBytesDemandedOnFrontOfRipeHash = 8;
    int64_t signingKeyNonce = 0;
    int64_t encryptionKeyNonce = 1;
    const EVP_MD *sha512 = EVP_sha512();
    // EVP_ripemd160() を使ってもプロバイダーを読み込まない限り使えない
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned char hash[64] = "";
    int nlz = 0;
    unsigned char signingPrivateKey[64];
    unsigned char encryptPrivateKey[64];
    PublicKey signing;
    PublicKey encrypt;
    char *address = NULL;
    char *signwif = NULL;
    char *encrwif = NULL;
    for (signingKeyNonce = 0, encryptionKeyNonce = 1;
         nlz < numberOfNullBytesDemandedOnFrontOfRipeHash;
         signingKeyNonce += 2, encryptionKeyNonce += 2)
    {
        deriviedPrivateKey(signingPrivateKey, argv[1], signingKeyNonce);
        deriviedPrivateKey(encryptPrivateKey, argv[1], encryptionKeyNonce);
        getPublicKey(&signing, (PrivateKey *)signingPrivateKey);
        getPublicKey(&encrypt, (PrivateKey *)encryptPrivateKey);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_DigestInit_ex2(ctx, sha512, NULL);
#else
        EVP_DigestInit_ex(ctx, sha512, NULL);
#endif
        EVP_DigestUpdate(ctx, signing, 65);
        EVP_DigestUpdate(ctx, encrypt, 65);
        EVP_DigestFinal_ex(ctx, hash, NULL);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_DigestInit_ex2(ctx, ripemd160, NULL);
#else
        EVP_DigestInit_ex(ctx, ripemd160, NULL);
#endif
        EVP_DigestUpdate(ctx, hash, 64);
        EVP_DigestFinal_ex(ctx, hash, NULL);
        if (hash[0] == 0)
        {
            address = encodeV4Address(hash, 20);
            signwif = encodeWIF((PrivateKey *)signingPrivateKey);
            encrwif = encodeWIF((PrivateKey *)encryptPrivateKey);
            printf("[%s]\n", address);
            printf("label = [chan] %s\n", argv[1]);
            printf("enabled = true\n");
            printf("decoy = false\n");
            printf("chan = true\n");
            printf("noncetrialsperbyte = 1000\n");
            printf("payloadlengthextrabytes = 1000\n");
            printf("signingKeyNonce = %ld\n", signingKeyNonce);
            printf("encryptionKeyNonce = %ld\n", encryptionKeyNonce);
            printf("privsigningkey = %s\n", signwif);
            printf("privencryptionkey = %s\n", encrwif);
            printf("\n");
            free(address);
            address = NULL;
            address = encodeV3Address(hash, 20);
            printf("[%s]\n", address);
            printf("label = [chan] %s\n", argv[1]);
            printf("enabled = true\n");
            printf("decoy = false\n");
            printf("chan = true\n");
            printf("noncetrialsperbyte = 1000\n");
            printf("payloadlengthextrabytes = 1000\n");
            printf("signingKeyNonce = %ld\n", signingKeyNonce);
            printf("encryptionKeyNonce = %ld\n", encryptionKeyNonce);
            printf("privsigningkey = %s\n", signwif);
            printf("privencryptionkey = %s\n", encrwif);
            printf("\n");
            free(signwif);
            signwif = NULL;
            free(encrwif);
            encrwif = NULL;
            break;
        }
    }

    EVP_MD_CTX_free(ctx);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);
#endif
    return 0;
}
