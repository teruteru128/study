
#include "java_random.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    int64_t seed = 0x0;
    struct drand48_data data;
    unsigned short buf[3];
    /*
    seed = lcgInverse(seed);
    printf("%ld\n", initialScramble(seed));
    seed = 107038380838084L;
    seed = initialScramble(seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = 246345201500483L;
    seed = initialScramble(seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = 240144965573432L;
    seed = initialScramble(seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = initialScramble(74803317123181L);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = initialScramble(74803317123181L);
    */
    /*
    buf[0] = (unsigned short)(seed >> 0);
    buf[1] = (unsigned short)(seed >> 16);
    buf[2] = (unsigned short)(seed >> 32);
    */
    /*
    memcpy(&buf[0], &seed, 6);
    seed48_r(buf, &data);
    printf("246 : %012lx\n", nextLong(&data));
    printf("246 : %012lx\n", nextLong(&data));
    memcpy(&buf[0], &seed, 6);
    seed48_r(buf, &data);
    printf("246 : %a\n", nextFloat(&data));
    printf("246 : %a\n", nextFloat(&data));
    memcpy(&buf[0], &seed, 6);
    seed48_r(buf, &data);
    printf("246 : %la\n", nextDouble(&data));
    printf("246 : %la\n", nextDouble(&data));
    */
    long random2 = 0;
    long random1;
    long work = 0L;
    for (random2 = 0x000000000000L; random2 <= 0x00000000ffffL; random2++)
    {
        printf("%012lx, ", random2);
        printf("%012lx, ", random1 = lcgInverse(random2));
        printf("%ld\n", initialScramble(lcgInverse(random1)));
    }
    return 0;
}
