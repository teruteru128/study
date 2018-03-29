
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <printint.h>

#define OUT_SIZE 21

int main(int argc, char** argv){
    uint64_t in = 10000000000000000000ULL;
    char out1[OUT_SIZE];
    char out2[OUT_SIZE];
    size_t i, j, length;
    memset(out1, 0, OUT_SIZE);
    memset(out2, 0, OUT_SIZE);
    for(i = 1; i <= 21; i++){
        length = snprintUInt64(out1, i, in);
        snprintf(out2, i, "%" PRIu64, in);
        printf("%" PRIu64 ", %20s(0x", in, out2);
        for(j = 0; j < 21; j++) {
            printf("%02x", out2[j]);
        }
        printf("), %20s(0x", out1);
        for(j = 0; j < length; j++) {
            printf("%02x", out1[j]);
        }
        printf("), %"PRIu64"\n", length);
    }
}
