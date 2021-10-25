
#include <bm.h>

int readprivkey(PrivateKey *pkey, size_t index)
{
    size_t fileindex = index >> 24;
    char filename[PATH_MAX];
    snprintf(filename, PATH_MAX, "privateKeys%ld.bin", fileindex);

    FILE *fin = fopen(filename, "rb");
    if (fin == NULL)
    {
        return EXIT_FAILURE;
    }

    long seekpos = (long)(index & 0xFFFFFFUL) * PRIVATE_KEY_LENGTH;

    if (fseek(fin, seekpos, SEEK_SET) != 0)
    {
        perror("fseek");
        fclose(fin);
        return EXIT_FAILURE;
    }

    if (fread(pkey, PRIVATE_KEY_LENGTH, 1, fin) != 1)
    {
        perror("fread");
        fclose(fin);
        return EXIT_FAILURE;
    }

    fclose(fin);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        return 1;
    }

    size_t signindex = strtoul(argv[1], NULL, 10);
    size_t encindex = strtoul(argv[2], NULL, 10);

    PrivateKey signprivkey = "";
    PrivateKey encprivkey = "";

    if (readprivkey(&signprivkey, signindex) != 0)
    {
        perror("readprivkey 1");
        return EXIT_FAILURE;
    }
    if (readprivkey(&encprivkey, encindex) != 0)
    {
        perror("readprivkey 2");
        return EXIT_FAILURE;
    }

    PublicKey signpubkey = "";
    PublicKey encpubkey = "";

    getPublicKey(&signpubkey, &signprivkey);
    getPublicKey(&encpubkey, &encprivkey);

    unsigned char hash[EVP_MAX_MD_SIZE];

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();

    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();

    EVP_DigestInit(ctx, sha512);
    EVP_DigestUpdate(ctx, signpubkey, PUBLIC_KEY_LENGTH);
    EVP_DigestUpdate(ctx, encpubkey, PUBLIC_KEY_LENGTH);
    EVP_DigestFinal(ctx, hash, NULL);
    EVP_DigestInit(ctx, ripemd160);
    EVP_DigestUpdate(ctx, hash, 64);
    EVP_DigestFinal(ctx, hash, NULL);

    EVP_MD_CTX_free(ctx);

    char *signwif = encodeWIF(&signprivkey);
    char *encwif = encodeWIF(&encprivkey);

    char *address = encodeV4Address(hash, 20);

    char *format = formatKey(address, signwif, encwif);

    free(address);
    printf("%s\n", format);
    free(format);

    return EXIT_SUCCESS;
}
