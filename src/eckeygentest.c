
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/objects.h>

#define PRIVATE_KEY_LENGTH 32
#define PUBLIC_KEY_LENGTH 65
#define MAX (9000)
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

    clock_gettime(CLOCK_REALTIME, &start);
    for (; i < MAX; i++)
    {
        r = EC_POINT_mul(secp256k1, pubkey, prikey, NULL, NULL, ctx);
        errchk(r, EC_POINT_mul);
        //r = EC_POINT_point2oct(secp256k1, pubkey, POINT_CONVERSION_UNCOMPRESSED, NULL, PUBLIC_KEY_LENGTH, ctx);
    }
    clock_gettime(CLOCK_REALTIME, &end);
    time_t diff = end.tv_sec - start.tv_sec;
    long ndiff = end.tv_nsec - start.tv_nsec;
    if(ndiff < 0)
    {
        ndiff += 1000000000;
        diff--;
    }
    double passed = ((double)diff * 1e9) + (double)ndiff;
    double seconds = passed / 1e9;
    fprintf(stderr, _("It took %.8f seconds.\n"), seconds);
    fprintf(stderr, _("%.1f times per second\n"), MAX / seconds);
    fprintf(stderr, _("%.12f seconds per time\n"), seconds / MAX);

    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    EC_POINT_free(pubkey);
    EC_GROUP_free(secp256k1);
    return EXIT_SUCCESS;
}
