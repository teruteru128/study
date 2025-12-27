
#ifndef LOAD_KEY_H
#define LOAD_KEY_H 1

#include <stdio.h>

int loadKey1(unsigned char *publicKey, const char *path, size_t size,
                    size_t num);
int loadPrivateKey1(unsigned char *publicKey, const char *path);
int loadPublicKey(unsigned char *publicKey, const char *path);
#endif
