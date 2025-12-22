
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <locale.h>
#include <sodium.h>
#if defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/random.h>
#endif

/**
 * */
int main(void)
{
    // https://libsodium.gitbook.io/doc/usage
#if defined(__linux__) && defined(RNDGETENTCNT)
    int fd;
    int c;

    if ((fd = open("/dev/random", O_RDONLY | O_CLOEXEC)) == -1)
    {
        perror("open /dev/random");
        return EXIT_FAILURE;
    }
    if (ioctl(fd, RNDGETENTCNT, &c) == 0 && c < 160)
    {
        fputs("This system doesn't provide enough entropy to quickly generate high-quality random numbers.\n"
                "Installing the rng-utils/rng-tools, jitterentropy or haveged packages may help.\n"
                "On virtualized Linux environments, also consider using virtio-rng.\n"
                "The service will not start until enough entropy has been collected.\n",
                stderr);
    }
    (void)close(fd);
    printf("%d\n", c);
#endif
    if (sodium_init() < 0)
    {
        return EXIT_FAILURE;
    }
    printf("%d, %d, %d\n", crypto_kx_PUBLICKEYBYTES, crypto_kx_SECRETKEYBYTES, crypto_kx_SESSIONKEYBYTES);
    unsigned char client_pk[crypto_kx_PUBLICKEYBYTES], client_sk[crypto_kx_SECRETKEYBYTES];
    unsigned char client_rx[crypto_kx_SESSIONKEYBYTES], client_tx[crypto_kx_SESSIONKEYBYTES];

    unsigned char server_pk[crypto_kx_PUBLICKEYBYTES], server_sk[crypto_kx_SECRETKEYBYTES];
    unsigned char server_rx[crypto_kx_SESSIONKEYBYTES], server_tx[crypto_kx_SESSIONKEYBYTES];

    /* Generate the client's key pair */
    if (crypto_kx_keypair(client_pk, client_sk))
    {
        perror("crypto_kx_keypair(cl)");
        return EXIT_FAILURE;
    }

    /* Generate the server's key pair */
    if (crypto_kx_keypair(server_pk, server_sk))
    {
        perror("crypto_kx_keypair(sv)");
        return EXIT_FAILURE;
    }

    /* Prerequisite after this point: the server's public key must be known by the client */

    /* Compute two shared keys using the server's public key and the client's secret key.
     client_rx will be used by the client to receive data from the server,
     client_tx will by used by the client to send data to the server. */
    if (crypto_kx_client_session_keys(client_rx, client_tx,
                                      client_pk, client_sk, server_pk))
    {
        /* Suspicious server public key, bail out */
        perror("crypto_kx_client_session_keys(cl)");
        return EXIT_FAILURE;
    }
    /* Prerequisite after this point: the client's public key must be known by the server */

    /* Compute two shared keys using the client's public key and the server's secret key.
     server_rx will be used by the server to receive data from the client,
     server_tx will by used by the server to send data to the client. */
    if (crypto_kx_server_session_keys(server_rx, server_tx,
                                      server_pk, server_sk, client_pk))
    {
        /* Suspicious client public key, bail out */
        perror("crypto_kx_client_session_keys(sv)");
        return EXIT_FAILURE;
    }
    int i = 0;
    printf("client_pk : ");
    for (i = 0; i < crypto_kx_PUBLICKEYBYTES; i++)
    {
        printf("%02x", client_pk[i]);
    }
    printf("\n");
    printf("client_sk : ");
    for (i = 0; i < crypto_kx_SECRETKEYBYTES; i++)
    {
        printf("%02x", client_sk[i]);
    }
    printf("\n");
    printf("server_pk : ");
    for (i = 0; i < crypto_kx_PUBLICKEYBYTES; i++)
    {
        printf("%02x", server_pk[i]);
    }
    printf("\n");
    printf("server_sk : ");
    for (i = 0; i < crypto_kx_SECRETKEYBYTES; i++)
    {
        printf("%02x", server_sk[i]);
    }
    printf("\n");
    char ctxstr[crypto_kx_SESSIONKEYBYTES * 2 + 1];
    for (i = 0; i < crypto_kx_SESSIONKEYBYTES; i++)
    {
        snprintf(&ctxstr[i << 1], 3, "%02x", client_tx[i]);
    }
    printf("client_tx : %s\n", ctxstr);
    for (i = 0; i < crypto_kx_SESSIONKEYBYTES; i++)
    {
        snprintf(&ctxstr[i << 1], 3, "%02x", server_rx[i]);
    }
    printf("server_rx : %s\n", ctxstr);
    printf("client_tx matches server_rx: %s\n", memcmp(client_tx, server_rx, crypto_kx_SESSIONKEYBYTES) == 0 ? "OK" : "NG");
    for (i = 0; i < crypto_kx_SESSIONKEYBYTES; i++)
    {
        snprintf(&ctxstr[i << 1], 3, "%02x", client_rx[i]);
    }
    printf("client_rx : %s\n", ctxstr);
    for (i = 0; i < crypto_kx_SESSIONKEYBYTES; i++)
    {
        snprintf(&ctxstr[i << 1], 3, "%02x", server_tx[i]);
    }
    printf("server_tx : %s\n", ctxstr);
    printf("client_rx matches server_tx: %s\n", memcmp(client_rx, server_tx, crypto_kx_SESSIONKEYBYTES) == 0 ? "OK" : "NG");
    return EXIT_SUCCESS;
}
