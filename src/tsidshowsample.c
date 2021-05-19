
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

#include <openssl/bio.h>
#include <openssl/asn1.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <assert.h>
#define USE_LTM
#define LTM_DESC
#include <tomcrypt.h>

#define STD_BUF_SIZE 0x1000

#define min(a, b) (((a) < (b)) ? (a) : (b))

static const char TSKEY[] = "b9dfaa7bee6ac57ac7b65f1094a1c155"
                            "e747327bc2fe5d51c512023fe54a2802"
                            "01004e90ad1daaae1075d53b7d571c30"
                            "e063b5a62a4a017bb394833aa0983e6e";

static void *safealloc(size_t len)
{
    void *result = calloc(len, sizeof(char));
    if (result == NULL)
    {
        printf("A memory allocation error occurred.\n");
        exit(EXIT_FAILURE);
    }

    return result;
}

static void safefree(void *ptr)
{
    if (ptr != NULL)
    {
        free(ptr);
    }
}

size_t calcDecodeLength(const char *b64input)
{ //Calculates the length of a decoded string
    size_t len = strlen(b64input);
    size_t padding = 0;
    //padding = (b64input[len - 1] == '=' ? (b64input[len - 2] == '=' ? 2 : 1) : 0);
    if (b64input[len - 1] == '=' && b64input[len - 2] == '=') //last two chars are =
        padding = 2;
    else if (b64input[len - 1] == '=') //last char is =
        padding = 1;

    return (len * 3) / 4 - padding;
}

static int deObfuscateInplace(unsigned char *data, uint32_t length)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    const EVP_MD *sha1 = EVP_sha1();
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_MD_CTX_free(mdctx);
    SHA_CTX ctx;
    if (SHA1_Init(&ctx) != 1)
    {
        return -1;
    }
    if (SHA1_Update(&ctx, data + 20, strlen((char *)(data + 20))) != 1)
    {
        return -1;
    }
    if (SHA1_Final(hash, &ctx) != 1)
    {
        return -1;
    }

    for (int i = 0; i < 20; i++)
    {
        data[i] ^= hash[i];
    }

    uint32_t dataSize = min(100, length);
    for (int i = 0; i < dataSize; i++)
    {
        data[i] ^= TSKEY[i];
    }
    return 0;
}

#define DigitTens ("0000000000000000" \
                   "1111111111111111" \
                   "2222222222222222" \
                   "3333333333333333" \
                   "4444444444444444" \
                   "5555555555555555" \
                   "6666666666666666" \
                   "7777777777777777" \
                   "8888888888888888" \
                   "9999999999999999" \
                   "aaaaaaaaaaaaaaaa" \
                   "bbbbbbbbbbbbbbbb" \
                   "cccccccccccccccc" \
                   "dddddddddddddddd" \
                   "eeeeeeeeeeeeeeee" \
                   "ffffffffffffffff")
#define DigitOnes ("0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef" \
                   "0123456789abcdef")

static int deObfuscateKey(const unsigned char *obfuscatedIdentity_base64,
                          ecc_key *ecckey)
{
    int ret = 0;
    unsigned char *eccKeyString = NULL;

    size_t actualIdentitySize = STD_BUF_SIZE;
    unsigned char *actualIdentity = safealloc(actualIdentitySize);

    if (base64_decode(obfuscatedIdentity_base64,
                      strlen((const char *)obfuscatedIdentity_base64),
                      actualIdentity,
                      &actualIdentitySize) != 0)
    {
        ret = -1;
        goto done;
    }
    if (deObfuscateInplace(actualIdentity, actualIdentitySize))
    {
        ret = -1;
        goto done;
    }

    size_t eccKeyStringSize = STD_BUF_SIZE;
    eccKeyString = safealloc(eccKeyStringSize);
    if (base64_decode(actualIdentity,
                      strlen((char *)actualIdentity),
                      eccKeyString,
                      &eccKeyStringSize) != 0)
    {
        ret = -1;
        goto done;
    }

    if (ecc_import(eccKeyString,
                   eccKeyStringSize,
                   ecckey) != CRYPT_OK)
    {
        ret = -1;
    }

done:
    safefree(actualIdentity);
    safefree(eccKeyString);

    return ret;
}

static int extractPublicKeyBase64(ecc_key *ecckey, unsigned char *out, long unsigned int *outlen)
{
    int ret = 0;

    size_t ecc_public_asn1_size = STD_BUF_SIZE;
    unsigned char *ecc_public_asn1 = safealloc(ecc_public_asn1_size);
    /* we export the public (!) key */
    if (ecc_export(ecc_public_asn1, &ecc_public_asn1_size, PK_PUBLIC, ecckey) != CRYPT_OK)
    {
        ret = -1;
        goto done;
    }

    if (base64_encode(ecc_public_asn1, ecc_public_asn1_size, out, outlen) != CRYPT_OK)
    {
        ret = -1;
        goto done;
    }

done:
    safefree(ecc_public_asn1);

    return ret;
}

static int parseIni(const char *filename,
                    unsigned char *out_identity,
                    size_t out_identity_len,
                    uint64_t *out_counter)
{
    int ret = 0;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        return -1;
    }

    char *filecontent = (char *)safealloc(STD_BUF_SIZE + 1);
    size_t newLen = fread(filecontent, sizeof(char), STD_BUF_SIZE, fp);
    if (newLen == 0)
    {
        printf("Error reading file.\n");
        ret = -1;
        goto done;
    }
    filecontent[++newLen] = '\0';

    const char *IDENT_STR = "identity";
    const char *currentpos = strstr(filecontent, IDENT_STR);
    if (currentpos == NULL)
    {
        ret = -1;
        goto done;
    }

    currentpos = (const char *)memchr(currentpos, '"', strlen(currentpos));
    if (currentpos == NULL)
    {
        ret = -1;
        goto done;
    }

    const char *counteridentity_startpos = currentpos + 1;
    const char *counteridentity_endpos = memchr(currentpos + 1,
                                                '"',
                                                strlen(currentpos + 1));
    if (counteridentity_endpos == NULL)
    {
        ret = -1;
        goto done;
    }
    const char *counterptr = counteridentity_startpos;

    const char *delimiter_pos = memchr(counteridentity_startpos,
                                       'V',
                                       strlen(counteridentity_startpos));
    if (delimiter_pos == NULL || delimiter_pos >= counteridentity_endpos)
    {
        ret = -1;
        goto done;
    }

    const char *identityptr = delimiter_pos + 1;

    size_t counter_len = delimiter_pos - counterptr;
    size_t identity_len = counteridentity_endpos - delimiter_pos - 1;
    if (identity_len > out_identity_len)
    {
        ret = -1;
        goto done;
    }

    // sanity checking counter
    for (size_t i = 0; i < counter_len; i++)
    {
        if (counterptr[i] < '0' || counterptr[i] > '9')
        {
            ret = -1;
            goto done;
        }
    }

    // sanity checking identity
    for (size_t i = 0; i < identity_len; i++)
    {
        if (!isprint(identityptr[i]) && !isspace(identityptr[i]))
        {
            ret = -1;
            goto done;
        }
    }

    strncpy((char *)out_identity, identityptr, identity_len);
    *out_counter = strtoull(counterptr, NULL, 10);

done:
    fclose(fp);
    safefree(filecontent);

    return ret;
}

static int getIDFingerprint(const char *publickey,
                            unsigned char *out,
                            long unsigned int *outlen)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    if (SHA1_Init(&ctx) != 1)
    {
        return 1;
    }
    if (SHA1_Update(&ctx, publickey, strlen(publickey)) != 1)
    {
        return 1;
    }
    if (SHA1_Final(hash, &ctx) != 1)
    {
        return 1;
    }

    if (base64_encode(hash, sizeof(hash) / sizeof(hash[0]), out, outlen) != 0)
    {
        return 1;
    }

    return 0;
}

static uint8_t getSecurityLevel(const char *publickey, uint64_t counter)
{
    size_t publickey_len = strlen(publickey);
    // a uint64_t takes at most 20 decimal digits
    size_t hashinput_len = publickey_len + 20 + 1;

    char *hashinput = (char *)safealloc(hashinput_len);
    size_t zerobytes = 0;
    size_t zerobits = 0;

    strncpy(hashinput, publickey, hashinput_len);
    int counter_len = snprintf(hashinput + publickey_len,
                               hashinput_len - publickey_len,
                               "%" PRIu64, counter);
    if (counter_len <= 0)
    {
        goto done;
    }

    unsigned char hash[20];
    SHA_CTX ctx;
    if (SHA1_Init(&ctx) != 1)
    {
        goto done;
    }
    if (SHA1_Update(&ctx,
                    (uint8_t *)hashinput,
                    publickey_len + (size_t)counter_len) != 1)
    {
        goto done;
    }
    if (SHA1_Final((uint8_t *)hash, &ctx) != 1)
    {
        goto done;
    }

    zerobytes = 0;
    while (zerobytes < 20 && hash[zerobytes] == 0)
    {
        zerobytes++;
    }
    zerobits = 0;
    if (zerobytes < 20)
    {
        char lastbyte = hash[zerobytes];
        while (!(lastbyte & 1))
        {
            zerobits++;
            lastbyte >>= 1;
        }
    }

done:
    safefree(hashinput);

    return 8 * zerobytes + zerobits;
}

static void readIdentity(char *filename)
{
    unsigned char *obfuscatedIdentity_base64 = (unsigned char *)safealloc(STD_BUF_SIZE);
    size_t counter;

    if (access(filename, F_OK) != 0)
    {
        printf("Error: The ini file does not exist.\n");
        exit(1);
    }

    if (access(filename, R_OK) != 0)
    {
        printf("Error: The ini file is not readable.\n");
        exit(1);
    }

    if (parseIni(filename, obfuscatedIdentity_base64, STD_BUF_SIZE, &counter))
    {
        printf("An error occurred while parsing the ini file.\n");
        exit(1);
    }

    ecc_key ecckey = {0, 0, NULL, {NULL, NULL, NULL}, NULL};
    if (deObfuscateKey(obfuscatedIdentity_base64, &ecckey))
    {
        printf("An error occurred while deobfuscating the identity.\n");
        exit(1);
    }

    size_t ecc_public_base64_size = STD_BUF_SIZE;
    char *ecc_public_base64 = safealloc(ecc_public_base64_size);

    if (extractPublicKeyBase64(&ecckey,
                               (unsigned char *)ecc_public_base64,
                               &ecc_public_base64_size))
    {
        printf("An error occurred while processing the obfuscated identity string.\n");
        exit(1);
    }

    long unsigned int idfingerprintlength = STD_BUF_SIZE;
    unsigned char *idfingerprint = safealloc(idfingerprintlength);

    if (getIDFingerprint(ecc_public_base64, idfingerprint, &idfingerprintlength))
    {
        printf("An error occurred while generating "
               "the fingerprint of the identity.\n");
        exit(1);
    }

    printf("Public key: %s\n", ecc_public_base64);
    printf("Public key length (Base64): %zd\n", strlen(ecc_public_base64));
    printf("Fingerprint: %s\n", idfingerprint);
    printf("Current security level: %" PRIu8 " (with counter=%" PRIu64 ")\n",
           getSecurityLevel(ecc_public_base64, counter), counter);

    safefree(ecc_public_base64);
    safefree(obfuscatedIdentity_base64);
    safefree(idfingerprint);
}

/**
 * @brief https://github.com/landave/TSIdentityTool
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char **argv)
{
    ltc_mp = ltm_desc;
    int rc = EXIT_SUCCESS;
    if (argc < 2)
    {
        printf("Usage : %s [ini file]\n", argv[0]);
    }
    else
    {
        readIdentity(argv[1]);
    }
    return rc;
}
