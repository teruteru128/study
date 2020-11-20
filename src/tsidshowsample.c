
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

#include <openssl/bio.h>
#include <openssl/asn1.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <assert.h>

#define STD_BUF_SIZE 0x1000
#define CRYPT_OK 0
#define PK_PUBLIC 0

#define min(a, b) (((a) < (b)) ? (a) : (b))

typedef struct ecc_point_t
{
    unsigned char *x;
    unsigned char *y;
    unsigned char *z;
} ecc_point;
typedef struct ecc_key_t
{
    int has_private_key;
    ecc_point public_key;
    unsigned char *private_key;
} ecc_key;

#define TSKEY ("b9dfaa7bee6ac57ac7b65f1094a1c155" \
               "e747327bc2fe5d51c512023fe54a2802" \
               "01004e90ad1daaae1075d53b7d571c30" \
               "e063b5a62a4a017bb394833aa0983e6e")

static void *safealloc(size_t len)
{
    void *result = calloc(1, len);
    if (result == NULL)
    {
        printf("A memory allocation error occurred.\n");
        exit(-1);
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
    size_t len = strlen(b64input),
           padding = 0;

    if (b64input[len - 1] == '=' && b64input[len - 2] == '=') //last two chars are =
        padding = 2;
    else if (b64input[len - 1] == '=') //last char is =
        padding = 1;

    return (len * 3) / 4 - padding;
}

int base64_decode(const char *in, size_t len, unsigned char **out, size_t *outlen)
{
    BIO *b64 = NULL;
    BIO *bio = NULL;
    size_t decodeLen = calcDecodeLength(in);
    *out = malloc(decodeLen + 1);
    memset(*out, 0, decodeLen + 1);

    bio = BIO_new_mem_buf(in, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    *outlen = BIO_read(bio, *out, strlen(in));
    BIO_free_all(bio);

    return 0;
}
int base64_encode(const unsigned char *in, size_t len, char **out, size_t *outlen)
{
    BIO *bio;
    BIO *b64;
    BUF_MEM *bufferPtr = NULL;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, out, len);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);
    *out = bufferPtr->data;
    *outlen = strlen(*out) + 1;
    return 0;
}
static int deObfuscateInplace(char *data, uint32_t length)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    if (SHA1_Init(&ctx) != 1)
    {
        return -1;
    }
    if (SHA1_Update(&ctx, data + 20, strlen(data + 20)) != 1)
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

    int dataSize = min(100, length);
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

int ecc_import(unsigned char *in, size_t inlen, ecc_key *key)
{
    /**
     * 
    */
    size_t i;
    //int has_private = 0;
    //unsigned char *public_x = NULL;
    //unsigned char *public_y = NULL;
    //unsigned char *private = NULL;
    for (i = 0; i < inlen; i++)
    {
        printf("%c%c", DigitTens[in[i]], DigitOnes[in[i]]);
    }
    printf("\n");
    key->has_private_key = 1;
    key->private_key = NULL;
    key->public_key.x = NULL;
    key->public_key.y = NULL;
    key->public_key.z = NULL;

    return 0;
}
int ecc_export(unsigned char *out, size_t *outlen, int type, ecc_key *key)
{
    return 0;
}
static int deObfuscateKey(const char *obfuscatedIdentity_base64,
                          ecc_key *ecckey)
{
    int ret = 0;

    char *actualIdentity = NULL;
    unsigned char *eccKeyString = NULL;

    size_t actualIdentitySize = 0;
    if (base64_decode(obfuscatedIdentity_base64,
                      strlen(obfuscatedIdentity_base64),
                      (unsigned char **)&actualIdentity,
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
    eccKeyString = NULL;
    if (base64_decode(actualIdentity,
                      strlen(actualIdentity),
                      &eccKeyString,
                      &eccKeyStringSize) != 0)
    {
        ret = -1;
        goto done;
    }

    if (ecc_import(eccKeyString, eccKeyStringSize, ecckey) != 0)
    {
        ret = -1;
    }

done:
    safefree(actualIdentity);
    safefree(eccKeyString);

    return ret;
}

static int extractPublicKeyBase64(ecc_key *ecckey,
                                  char **out,
                                  long unsigned int *outlen)
{
    int ret = 0;

    uint64_t ecc_public_asn1_size = STD_BUF_SIZE;
    char *ecc_public_asn1 = (char *)safealloc(ecc_public_asn1_size);
    if (ecc_export((uint8_t *)ecc_public_asn1,
                   &ecc_public_asn1_size,
                   PK_PUBLIC /* we export the public (!) key */,
                   ecckey) != CRYPT_OK)
    {
        ret = -1;
        goto done;
    }

    if (base64_encode((uint8_t *)ecc_public_asn1,
                      ecc_public_asn1_size,
                      out, outlen) != CRYPT_OK)
    {
        ret = -1;
        goto done;
    }

done:
    safefree(ecc_public_asn1);

    return ret;
}

static int parseIni(const char *filename,
                    char *out_identity,
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

    strncpy(out_identity, identityptr, identity_len);
    *out_counter = strtoull(counterptr, NULL, 10);

done:
    fclose(fp);
    safefree(filecontent);

    return ret;
}

static int getIDFingerprint(const char *publickey,
                            char **out,
                            long unsigned int *outlen)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    if (SHA1_Init(&ctx) != 1)
    {
        return 1;
    }
    if (SHA1_Update(&ctx,
                    publickey,
                    strlen(publickey)) != 1)
    {
        return 1;
    }
    if (SHA1_Final(hash, &ctx) != 1)
    {
        return 1;
    }

    if (base64_encode(hash,
                      sizeof(hash) / sizeof(hash[0]),
                      out, outlen) != 0)
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

    char hash[20];
    SHA_CTX ctx;
    if (SHA1_Init(&ctx) != CRYPT_OK)
    {
        goto done;
    }
    if (SHA1_Update(&ctx,
                    (uint8_t *)hashinput,
                    publickey_len + (size_t)counter_len) != CRYPT_OK)
    {
        goto done;
    }
    if (SHA1_Final((uint8_t *)hash, &ctx) != CRYPT_OK)
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
        uint8_t lastbyte = hash[zerobytes];
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
    char *obfuscatedIdentity_base64 = (char *)safealloc(STD_BUF_SIZE);
    uint64_t counter;

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

    if (parseIni(filename,
                 obfuscatedIdentity_base64,
                 STD_BUF_SIZE, &counter))
    {
        printf("An error occurred while parsing the ini file.\n");
        exit(1);
    }

    ecc_key *ecckey = (ecc_key *)safealloc(sizeof(ecc_key));
    if (deObfuscateKey(obfuscatedIdentity_base64, ecckey))
    {
        printf("An error occurred while deobfuscating the identity.\n");
        exit(1);
    }

    uint64_t ecc_public_base64_size = 0;
    char *ecc_public_base64 = NULL;

    if (extractPublicKeyBase64(ecckey,
                               &ecc_public_base64,
                               &ecc_public_base64_size))
    {
        printf("An error occurred while processing "
               "the obfuscated identity string.\n");
        exit(1);
    }

    long unsigned int idfingerprintlength = STD_BUF_SIZE;
    char *idfingerprint = NULL;

    if (getIDFingerprint(ecc_public_base64,
                         &idfingerprint,
                         &idfingerprintlength))
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
    safefree(ecckey);
    safefree(idfingerprint);
}

int main(int argc, char **argv)
{
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
