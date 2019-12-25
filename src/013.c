
#include <config.h>
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
  if(sodium_init() < 0){
    return EXIT_FAILURE;
  }
  unsigned char client_pk[crypto_kx_PUBLICKEYBYTES], client_sk[crypto_kx_SECRETKEYBYTES];
  unsigned char client_rx[crypto_kx_SESSIONKEYBYTES], client_tx[crypto_kx_SESSIONKEYBYTES];

  unsigned char server_pk[crypto_kx_PUBLICKEYBYTES], server_sk[crypto_kx_SECRETKEYBYTES];
  unsigned char server_rx[crypto_kx_SESSIONKEYBYTES], server_tx[crypto_kx_SESSIONKEYBYTES];

  /* Generate the client's key pair */
  crypto_kx_keypair(client_pk, client_sk);

  /* Generate the server's key pair */
  crypto_kx_keypair(server_pk, server_sk);

  /* Prerequisite after this point: the server's public key must be known by the client */

  /* Compute two shared keys using the server's public key and the client's secret key.
     client_rx will be used by the client to receive data from the server,
     client_tx will by used by the client to send data to the server. */
  if (crypto_kx_client_session_keys(client_rx, client_tx,
        client_pk, client_sk, server_pk) != 0) {
    /* Suspicious server public key, bail out */
  }
  /* Prerequisite after this point: the client's public key must be known by the server */

  /* Compute two shared keys using the client's public key and the server's secret key.
     server_rx will be used by the server to receive data from the client,
     server_tx will by used by the server to send data to the client. */
  if (crypto_kx_server_session_keys(server_rx, server_tx,
        server_pk, server_sk, client_pk) != 0) {
    /* Suspicious client public key, bail out */
  }
  char const *bmap[] ={
  "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0a", "0b", "0c", "0d", "0e", "0f",
  "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1a", "1b", "1c", "1d", "1e", "1f",
  "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2a", "2b", "2c", "2d", "2e", "2f",
  "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3a", "3b", "3c", "3d", "3e", "3f",
  "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4a", "4b", "4c", "4d", "4e", "4f",
  "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5a", "5b", "5c", "5d", "5e", "5f",
  "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6a", "6b", "6c", "6d", "6e", "6f",
  "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7a", "7b", "7c", "7d", "7e", "7f",
  "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8a", "8b", "8c", "8d", "8e", "8f",
  "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9a", "9b", "9c", "9d", "9e", "9f",
  "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "aa", "ab", "ac", "ad", "ae", "af",
  "b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", "b9", "ba", "bb", "bc", "bd", "be", "bf",
  "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8", "c9", "ca", "cb", "cc", "cd", "ce", "cf",
  "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "da", "db", "dc", "dd", "de", "df",
  "e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7", "e8", "e9", "ea", "eb", "ec", "ed", "ee", "ef",
  "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "fa", "fb", "fc", "fd", "fe", "ff",
  };
  int i;
  printf("client_tx : ");
  for(i = 0; i<crypto_kx_SESSIONKEYBYTES;i++){
    printf("%s", bmap[client_tx[i]]);
  }
  printf("\n");
  printf("server_rx : ");
  for(i = 0; i<crypto_kx_SESSIONKEYBYTES;i++){
    printf("%s", bmap[server_rx[i]]);
  }
  printf("\n");
  printf("client_rx : ");
  for(i = 0; i<crypto_kx_SESSIONKEYBYTES;i++){
    printf("%s", bmap[client_rx[i]]);
  }
  printf("\n");
  printf("server_tx : ");
  for(i = 0; i<crypto_kx_SESSIONKEYBYTES;i++){
    printf("%s", bmap[server_tx[i]]);
  }
  printf("\n");
  return EXIT_SUCCESS;
}
