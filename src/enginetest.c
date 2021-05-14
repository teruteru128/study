
#include "config.h"
#include <openssl/engine.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    /* Load all bundled ENGINEs into memory and make them visible */
    ENGINE_load_builtin_engines();
    /* Register all of them for every algorithm they collectively implement */
    ENGINE_register_all_complete();
    ENGINE *e = ENGINE_get_default_RAND();
    if (e != NULL)
    {
        printf("%s\n", ENGINE_get_id(e));
    }
    else
    {
        printf("NULL\n");
    }
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
    return 0;
}
