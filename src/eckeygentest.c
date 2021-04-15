
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include "timeutil.h"

#define PRIVATE_KEY_LENGTH 32
#define PUBLIC_KEY_LENGTH 65
#define MAX 10000
#define errchk(v, f)                                                \
    if (!v)                                                         \
    {                                                               \
        unsigned long err = ERR_get_error();                        \
        fprintf(stderr, #f " : %s\n", ERR_error_string(err, NULL)); \
        return EXIT_FAILURE;                                        \
    }

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    // curve 生成
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    //const EC_POINT *g = EC_GROUP_get0_generator(secp256k1);
    //const EC_METHOD *secp256r1 = EC_GFp_nistp256_method();
    EC_POINT *pubkey = EC_POINT_new(secp256k1);
    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    BIGNUM *prikey = BN_CTX_get(ctx);
    const BIGNUM *n = EC_GROUP_get0_order(secp256k1);

    BN_copy(prikey, n);
    BN_sub_word(prikey, 1);

    int r = 0;
    int i = 0;
    struct timespec start;
    struct timespec end;
    struct timespec d;

    clock_gettime(CLOCK_REALTIME, &start);
    for (; i < MAX; i++)
    {
        r = EC_POINT_mul(secp256k1, pubkey, prikey, NULL, NULL, ctx);
        errchk(r, EC_POINT_mul);
        //r = EC_POINT_point2oct(secp256k1, pubkey, POINT_CONVERSION_UNCOMPRESSED, NULL, PUBLIC_KEY_LENGTH, ctx);
    }
    clock_gettime(CLOCK_REALTIME, &end);
    difftimespec(&d, &end, &start);
    double diff = difftime(end.tv_sec, start.tv_sec);
    long ndiff = end.tv_nsec - start.tv_nsec;
    if (ndiff < 0)
    {
        ndiff += 1000000000;
        diff--;
    }
    double seconds = fma((double)d.tv_sec, 1e9, (double)d.tv_nsec) / 1e9;
    fprintf(stderr, _("It took %ld.%09ld seconds.\n"), d.tv_sec, d.tv_nsec);
    fprintf(stderr, _("%.1f times per second\n"), MAX / seconds);
    fprintf(stderr, _("%.12f seconds per time\n"), seconds / MAX);
    printf("%ld.%09ld\n", (long)diff, ndiff);
    printf("%.0f.%09ld\n", diff, ndiff);

    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    EC_POINT_free(pubkey);
    EC_GROUP_free(secp256k1);
    return EXIT_SUCCESS;
}
