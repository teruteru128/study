
#include <stdio.h>
#include <inttypes.h>

int main(int argc, char** argv) {
    uint64_t in = 12345678901234567890ULL;
    char out[25];
    size_t i;
    for(i = 0; i <= 25; i++) {
        out[0] = 0;
        snprintf(out, i, "%" PRIu64, in);
        printf("%"PRIu64", %s\n", i, out);
    }
}
