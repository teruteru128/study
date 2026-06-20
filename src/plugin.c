
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <plugin.h>
#include <dlfcn.h>

void *aaaa_aaaa(void *message) {
    printf("[Optional Feature] Plugin running: %s\n", (char *)message);
    return NULL;
}

