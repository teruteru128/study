
#include <stdio.h>
#include <stdlib.h>
#include <openssl/bn.h>

#define N "af240808297a359e600caae74b3b4edc7cbc3c451cbb2be0fe2902f95708a364" \
          "851527f5f1adc831895d22e82aaaa642b38ff8b955b7b1b74bb3fe8f7e0757ec" \
          "ef43db66621561cf600da4d8def8e0c362083d5413eb49ca59548526e52b8f1b" \
          "9febf5a191c23349d843636a524bd28fe870514dd189697bc770f6b3dc1274db" \
          "7b5d4b56d396bf1577a1b0f4a225f2af1c926718e5f40604ef90b9e400e4dd3a" \
          "b519ff02baf43ceee08beb378becf4d7acf2f6f03dafdd759133191d1c40cb74" \
          "24192193d914feac2a52c78fd50449e48d6347883c6983cbfe47bd2b7e4fc595" \
          "ae0e9dd4d143c06773e314087ee53f9f73b8330acf5d3f3487968aee53e82515"

/**
 * collatz huge integer test
 * */
int main(int argc, char **argv)
{
    int success = 0;
    BIGNUM *n = BN_new();
    if (!n)
    {
        return EXIT_FAILURE;
    }
    BIGNUM *work = n;
    BIGNUM *tmp = BN_new();
    if (!tmp)
    {
        return EXIT_FAILURE;
    }
    const BIGNUM *one = BN_value_one();
    if (!one)
    {
        return EXIT_FAILURE;
    }
    BN_CTX *ctx = BN_CTX_new();
    if (!ctx)
    {
        return EXIT_FAILURE;
    }

    success = BN_hex2bn(&work, N);
    if (!success)
    {
        perror(NULL);
        return EXIT_FAILURE;
    }

    work = BN_copy(tmp, n);
    if (!work)
    {
        perror(NULL);
        return EXIT_FAILURE;
    }

    char *out = NULL;
    while (BN_cmp(tmp, one) > 0)
    {
        if (BN_is_odd(tmp))
        {
            // odd
            success = BN_mul_word(tmp, 3);
            if (!success)
            {
                perror(NULL);
                return EXIT_FAILURE;
            }
            success = BN_add_word(tmp, 1);
            if (!success)
            {
                perror(NULL);
                return EXIT_FAILURE;
            }
        }
        else
        {
            //even
            success = BN_rshift(tmp, tmp, 1);
            if (!success)
            {
                perror(NULL);
                return EXIT_FAILURE;
            }
        }
        out = BN_bn2dec(tmp);
        printf("%s\n", out);
        OPENSSL_free(out);
        out = NULL;
    }

    BN_free(n);
    BN_free(tmp);
    BN_CTX_free(ctx);

    return EXIT_SUCCESS;
}
