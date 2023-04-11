
#define _GNU_SOURCE
#include <limits.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/random.h>
#include <unistd.h>
#include <uuid/uuid.h>

static unsigned char privatekey_raw[32];
pthread_mutex_t prikeyraw_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t prikeyraw_cond = PTHREAD_COND_INITIALIZER;

void *keyreadthread(void *b)
{
    ssize_t numberOfRandomBytes = 0;
    for (size_t i = 0; i < 16777216UL; i++)
    {
        pthread_mutex_lock(&prikeyraw_mutex);
        numberOfRandomBytes = getrandom(privatekey_raw, 32, GRND_RANDOM);
        if (numberOfRandomBytes >= 0)
        {
            pthread_cond_broadcast(&prikeyraw_cond);
        }
        pthread_mutex_unlock(&prikeyraw_mutex);
        if (numberOfRandomBytes < 0)
        {
            break;
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{

    uuid_t uuid;
    char uuidstr[UUID_STR_LEN];
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuidstr);
    char pubkeypath[PATH_MAX];
    char prikeypath[PATH_MAX];
    snprintf(pubkeypath, PATH_MAX, "publickey-%s.bin", uuidstr);
    snprintf(prikeypath, PATH_MAX, "privatekey-%s.bin", uuidstr);
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    EC_POINT *pubkey = EC_POINT_new(secp256k1);
    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    BIGNUM *prikey = BN_CTX_get(ctx);
    const BIGNUM *n = EC_GROUP_get0_order(secp256k1);

    unsigned char p[65];
    FILE *pubkeyfile = fopen(pubkeypath, "ab");
    FILE *prikeyfile = fopen(prikeypath, "ab");

    pthread_t thread;
    pthread_create(&thread, NULL, keyreadthread, NULL);

    for (size_t i = 0; i < 16777216; i++)
    {
        pthread_mutex_lock(&prikeyraw_mutex);
        pthread_cond_wait(&prikeyraw_cond, &prikeyraw_mutex);
        BN_bin2bn(privatekey_raw, 32, prikey);
        pthread_mutex_unlock(&prikeyraw_mutex);
        EC_POINT_mul(secp256k1, pubkey, prikey, NULL, NULL, ctx);
        EC_POINT_point2oct(secp256k1, pubkey, POINT_CONVERSION_UNCOMPRESSED, p,
                           65, ctx);
        fwrite(privatekey_raw, 32, 1, prikeyfile);
        fwrite(p, 65, 1, pubkeyfile);
    }

    fclose(prikeyfile);
    fclose(pubkeyfile);
    pthread_join(thread, NULL);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    EC_POINT_free(pubkey);
    EC_GROUP_free(secp256k1);

    return 0;
}
