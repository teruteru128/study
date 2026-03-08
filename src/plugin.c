
#include <stdio.h>
#include <plugin.h>

void *aaaa_aaaa(void *message) {
    printf("[Optional Feature] Plugin running: %s\n", (char *)message);
    return (void *) 0ULL;
}

