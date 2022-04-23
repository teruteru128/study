
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <bitmessage.h>
#include <bm.h>
#include <locale.h>
#include <openssl/evp.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>

#define HEXTABLE "0123456789abcdef"

static char *buildTransmitdata(unsigned char *signpub, size_t signpublen,
                               unsigned char *encpub, size_t encpublen)
{
    assert(signpublen == 65);
    assert(encpublen == 65);
    char *ret = malloc(281);
    // 16進数エンコードめんどくさいんじゃボケェ
    // address version
    ret[0] = '0';
    ret[1] = '4';
    // stream number
    ret[2] = '0';
    ret[3] = '1';
    // bitfield
    ret[4] = '0';
    ret[5] = '0';
    ret[6] = '0';
    ret[7] = '0';
    ret[8] = '0';
    ret[9] = '0';
    ret[10] = '0';
    ret[11] = '1';
    // signing key
    for (size_t i = 0; i < 64; i++)
    {
        ret[(i << 1) + 12] = HEXTABLE[(signpub[i + 1] >> 4) & 0x0f];
        ret[(i << 1) + 13] = HEXTABLE[(signpub[i + 1] >> 0) & 0x0f];
    }
    // encrypting key
    for (size_t i = 0; i < 64; i++)
    {
        ret[(i << 1) + 140] = HEXTABLE[(encpub[i + 1] >> 4) & 0x0f];
        ret[(i << 1) + 141] = HEXTABLE[(encpub[i + 1] >> 0) & 0x0f];
    }
    // 140 + 128
    // FD03E8
    // noncetrialsperbyte
    ret[268] = 'f';
    ret[269] = 'd';
    ret[270] = '0';
    ret[271] = '3';
    ret[272] = 'e';
    ret[273] = '8';
    // payloadlengthextrabytes
    ret[274] = 'f';
    ret[275] = 'd';
    ret[276] = '0';
    ret[277] = '3';
    ret[278] = 'e';
    ret[279] = '8';
    ret[280] = '\0';

    return ret;
}

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL, "");
    unsigned char *publickeys = calloc(67108864UL, 65UL);
    {
        FILE *fin = fopen("publicKeys.bin", "rb");
        if (fin == NULL)
        {
            perror("fopen");
            return 1;
        }
        size_t objnum = fread(publickeys, 65, 67108864, fin);
        if (objnum != 67108864)
        {
            perror("fread");
            fclose(fin);
            free(publickeys);
            return EXIT_FAILURE;
        }
        fclose(fin);
    }
    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    FILE *addresstxt = fopen("address.txt", "wa");
    FILE *insertsql = fopen("insert.sql", "wa");
    if (addresstxt == NULL || insertsql == NULL)
    {
        perror("");
        return EXIT_FAILURE;
    }
    // CREATE TABLE pubkeys (address text, addressversion int, transmitdata
    // blob, time int, usedpersonally text, UNIQUE(address) ON CONFLICT
    // REPLACE);
    fputs("insert into pubkeys(address, addressversion, transmitdata, time, "
          "usedpersonally) values",
          insertsql);
    size_t i = 0;
    size_t j = 0;
    unsigned char work[EVP_MAX_MD_SIZE] = "";
    char *address = NULL;
    char *transmitdatahex = NULL;
    int isFirst = 1;
    size_t k = 0;
    unsigned char *signpubkey = NULL;
    /*
        0,     65
       65,    130
      130,    195
      195,    260
      260,    325
    */
    for (i = 260; i < 325; i += 65)
    {
        signpubkey = publickeys + i;
        for (j = 0; j < 4362076160UL; j += 65)
        {
            EVP_DigestInit(mdctx, sha512);
            EVP_DigestUpdate(mdctx, signpubkey, 65);
            EVP_DigestUpdate(mdctx, publickeys + j, 65);
            EVP_DigestFinal(mdctx, work, NULL);
            EVP_DigestInit(mdctx, ripemd160);
            EVP_DigestInit(mdctx, ripemd160);
            EVP_DigestUpdate(mdctx, work, 64);
            EVP_DigestFinal(mdctx, work, NULL);
            if (work[0] == 0)
            {
                address = encodeV4Address(work, 20);
                transmitdatahex
                    = buildTransmitdata(signpubkey, 65, publickeys + j, 65);
                fprintf(addresstxt, "%s\n", address);
                if (isFirst == 1)
                {
                    isFirst = 0;
                }
                else
                {
                    fputs(",", insertsql);
                }
                fprintf(insertsql, "\n('%s', 4, x'%s', %ld, 'yes')", address,
                        transmitdatahex, time(NULL));
                free(address);
                free(transmitdatahex);
                if ((k % 1000) == 999)
                {
                    fputs(";\nINSERT INTO pubkeys(address, addressversion, "
                          "transmitdata, time, usedpersonally) values",
                          insertsql);
                    isFirst = 1;
                }
                k++;
            }
        }
    }

    fputs(";\n", insertsql);
    fclose(insertsql);
    fclose(addresstxt);
    free(publickeys);
    return 0;
}
