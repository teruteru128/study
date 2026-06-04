
#include "java_random.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    int64_t seed = 0x0;
    struct drand48_data data;
    unsigned short buf[3];
    seed = lcgInverse(seed);
    printf("%ld\n", initialScramble(seed));
    seed = lcgInverse(0);
    seed = lcgInverse(seed);
    printf("%ld\n", initialScramble(seed));
    printf("--\n");
    seed = 107038380838084L;
    seed = initialScramble(seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    printf("--\n");
    seed = 246345201500483L;
    seed = initialScramble(seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    printf("--\n");
    seed = 240144965573432L;
    seed = initialScramble(seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    printf("--\n");
    seed = initialScramble(74803317123181L);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = lcg(seed);
    printf("246 : %012lx\n", seed);
    seed = initialScramble(74803317123181L);
    /*
    buf[0] = (unsigned short)(seed >> 0);
    buf[1] = (unsigned short)(seed >> 16);
    buf[2] = (unsigned short)(seed >> 32);
    */
    memcpy(&buf[0], &seed, 6);
    seed48_r(buf, &data);
    printf("long : %012lx\n", nextLong(&data));
    printf("long : %012lx\n", nextLong(&data));
    memcpy(&buf[0], &seed, 6);
    seed48_r(buf, &data);
    printf("float : %f\n", nextFloat(&data));
    float b = nextFloat(&data);
    int work2;
    memcpy(&work2, &b, sizeof(float));
    printf("float : %f, %08x\n", b, work2);
    printf("float : %f\n", nextFloat(&data));
    printf("float : %f\n", nextFloat(&data));
    memcpy(&buf[0], &seed, 6);
    seed48_r(buf, &data);
    printf("double : %lf\n", nextDouble(&data));
    printf("double : %lf\n", nextDouble(&data));
    printf("double : %lf\n", nextDouble(&data));
    printf("double : %lf\n", nextDouble(&data));
    memcpy(&buf[0], &seed, 6);
    seed48_r(buf, &data);
    printf("int : %08x\n", nextInt(&data));
    printf("int : %08x\n", nextInt(&data));
    printf("int : %08x\n", nextInt(&data));
    printf("int : %08x\n", nextInt(&data));
    /*
    long pref = 0xffffffc00000L;
    long random2 = 0;
    long random1;
    long work = 0L;
    long count = 0;
    for (count = 0x0L; count < 0x400000L; count++)
    {
        random2 = pref + count;
        printf("%012lx, ", random2);
        printf("%012lx, ", random1 = lcgInverse(random2));
        printf("%ld\n", initialScramble(lcgInverse(random1)));
    }
    */
    seed = initialScramble(178082884934598L);
    memcpy(&buf[0], &seed, 6);
    seed48_r(buf, &data);
    printf("246 : %a\n", nextFloat(&data));
    printf("246 : %a\n", nextFloat(&data));
    printf("246 : %a\n", nextFloat(&data));
    printf("246 : %a\n", nextFloat(&data));
    seed = initialScramble(107038380838084L);
    memcpy(&buf[0], &seed, 6);
    seed48_r(buf, &data);
    printf("246 : %a\n", nextFloat(&data));
    printf("246 : %a\n", nextFloat(&data));
    printf("246 : %a\n", nextFloat(&data));
    printf("246 : %a\n", nextFloat(&data));

    return 0;
}
