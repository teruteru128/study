/* ftm.c - from feature_test_macros(7) */

#define _GNU_SOURCE
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define p(mac) printf(#mac " defined\n")

int main(int argc, char *argv[])
{
#ifdef _POSIX_SOURCE
    p(_POSIX_SOURCE);
#endif

#ifdef _POSIX_C_SOURCE
    printf("_POSIX_C_SOURCE defined: %ldL\n", _POSIX_C_SOURCE);
#endif

#ifdef _ISOC99_SOURCE
    p(_ISOC99_SOURCE);
#endif

#ifdef _ISOC11_SOURCE
    p(_ISOC11_SOURCE);
#endif

#ifdef _XOPEN_SOURCE
    printf("_XOPEN_SOURCE defined: %d\n", _XOPEN_SOURCE);
#endif

#ifdef _XOPEN_SOURCE_EXTENDED
    p(_XOPEN_SOURCE_EXTENDED);
#endif

#ifdef _LARGEFILE64_SOURCE
    p(_LARGEFILE64_SOURCE);
#endif

#ifdef _LARGEFILE_SOURCE
    p(_LARGEFILE_SOURCE);
#endif

#ifdef _FILE_OFFSET_BITS
    printf("_FILE_OFFSET_BITS defined: %d\n", _FILE_OFFSET_BITS);
#endif

#ifdef _BSD_SOURCE
    p(_BSD_SOURCE);
#endif

#ifdef _SVID_SOURCE
    p(_SVID_SOURCE);
#endif

#ifdef _DEFAULT_SOURCE
    p(_DEFAULT_SOURCE);
#endif

#ifdef _ATFILE_SOURCE
    p(_ATFILE_SOURCE);
#endif

#ifdef _GNU_SOURCE
    p(_GNU_SOURCE);
#endif

#ifdef _REENTRANT
    p(_REENTRANT);
#endif

#ifdef _THREAD_SAFE
    p(_THREAD_SAFE);
#endif

#ifdef _FORTIFY_SOURCE
    printf("_FORTIFY_SOURCE defined: %d\n", _FORTIFY_SOURCE);
#endif

#ifdef BYTE_ORDER
    p(BYTE_ORDER);
#if BYTE_ORDER == LITTLE_ENDIAN
    printf("BYTE_ORDER is LITTLE_ENDIAN\n");
#elif BYTE_ORDER == BIG_ENDIAN
    printf("BYTE_ORDER is BIG_ENDIAN\n");
#elif BYTE_ORDER == PDP_ENDIAN
    printf("BYTE_ORDER is PDP_ENDIAN\n");
#else
    printf("BYTE_ORDER is UNKNOWN ENDIAN\n");
#endif
#endif

    return EXIT_SUCCESS;
}
