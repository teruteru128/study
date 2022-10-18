
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <openssl/engine.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/provider.h>
#endif
#include <stdio.h>

int main(int argc, char **argv)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER *def;
    OSSL_PROVIDER *legacy;
    OSSL_PROVIDER *fips;
    OSSL_PROVIDER *base;
    OSSL_PROVIDER *nul;
    legacy = OSSL_PROVIDER_load(NULL, "legacy");
    if (legacy == NULL)
    {
        printf("Failed to load Legacy provider\n");
    }
    else
    {
        printf("legacy provider OK...\n");
    }
    def = OSSL_PROVIDER_load(NULL, "default");
    if (def == NULL)
    {
        printf("Failed to load Default provider\n");
    }
    else
    {
        printf("default provider OK...\n");
    }
    fips = OSSL_PROVIDER_load(NULL, "fips");
    if (fips == NULL)
    {
        printf("Failed to load FIPS provider\n");
    }
    else
    {
        printf("fips provider OK...\n");
    }
    base = OSSL_PROVIDER_load(NULL, "base");
    if (base == NULL)
    {
        printf("Failed to load Base provider\n");
    }
    else
    {
        printf("base provider OK...\n");
    }
    nul = OSSL_PROVIDER_load(NULL, "null");
    if (def == NULL)
    {
        printf("Failed to load Null provider\n");
    }
    else
    {
        printf("null provider OK...\n");
    }
    printf("OK!\n");
    if (def != NULL)
        OSSL_PROVIDER_unload(def);
    if (legacy != NULL)
        OSSL_PROVIDER_unload(legacy);
    if (fips != NULL)
        OSSL_PROVIDER_unload(fips);
    if (base != NULL)
        OSSL_PROVIDER_unload(base);
    if (nul != NULL)
        OSSL_PROVIDER_unload(nul);
#else
    /* Load all bundled ENGINEs into memory and make them visible */
    ENGINE_load_builtin_engines();
    /* Register all of them for every algorithm they collectively implement */
    ENGINE_register_all_complete();
    ENGINE *e = ENGINE_get_default_RAND();
    if (e != NULL)
    {
        printf("default : %s\n", ENGINE_get_id(e));
    }
    else
    {
        printf("default : NULL\n");
    }
    printf("--\n");
    ENGINE *first = ENGINE_get_first();
    if (first != NULL)
    {
        /*
        for (ENGINE *engine = first;
             (engine = ENGINE_get_next(engine)) != NULL;)
        {
            printf("%s\n", ENGINE_get_id(engine));
        }
        */
        while (first != NULL)
        {
            printf("%s\n", ENGINE_get_id(first));
            first = ENGINE_get_next(first);
        }
    }
    else
    {
        printf("first not found!\n");
    }
    ENGINE *rdrand = ENGINE_by_id("rdrand");
    if (rdrand != NULL)
    {
        printf("%s\n", ENGINE_get_id(rdrand));
    }
#endif
    return 0;
}
