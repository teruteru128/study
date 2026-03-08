
#include <ctype.h>
#include <stdio.h>

int main() {
    for(int i = 0x20; i < 0x7f; i++) {
        printf("isprint(%c) -> %d\n", i, isprint(i));
    }
    return 0;
}

