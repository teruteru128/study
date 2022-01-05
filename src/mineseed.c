
#include "java_random.h"
#include <stdio.h>

int main(int argc, char const *argv[])
{
    int64_t seed = 0x0;
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
     */
    seed = initialScramble(74803317123181L);
    printf("246 : %012lx\n", nextLong(&seed));
    printf("246 : %012lx\n", nextLong(&seed));
    long work = 0L;
    for (seed = 0x000000000000L; seed <= 0x00000000ffffL; seed++)
    {
        printf("%012lx, ", seed);
        printf("%012lx, ", work = lcgInverse(seed));
        printf("%ld\n", initialScramble(lcgInverse(work)));
    }
    return 0;
}
