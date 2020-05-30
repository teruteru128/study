
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <locale.h>
#include <sodium.h>

/**
 * locale test
 * */
int main(int argc, char **argv)
{
  char *locale = setlocale(LC_ALL, "");
  printf("%s\n", locale);
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
  if(crypto_kx_keypair(server_pk, server_sk))
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
  char ctxstr[65];
  printf("client_tx : ");
  for (i = 0; i < crypto_kx_SESSIONKEYBYTES; i++)
  {
    snprintf(&ctxstr[i * 2], 3, "%02x", client_tx[i]);
  }
  printf("%s\n", ctxstr);
  printf("server_rx : ");
  for (i = 0; i < crypto_kx_SESSIONKEYBYTES; i++)
  {
    printf("%02x", server_rx[i]);
  }
  printf("\n");
  printf("client_rx : ");
  for (i = 0; i < crypto_kx_SESSIONKEYBYTES; i++)
  {
    printf("%02x", client_rx[i]);
  }
  printf("\n");
  printf("server_tx : ");
  for (i = 0; i < crypto_kx_SESSIONKEYBYTES; i++)
  {
    printf("%02x", server_tx[i]);
  }
  printf("\n");
  return EXIT_SUCCESS;
}
