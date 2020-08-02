
#include <stdio.h>
#include "java_random.h"

int main(int argc, char const *argv[])
{
  int64_t ppp = 0x0;
  ppp = nInverse(ppp);
  printf("%ld\n", initialScramble(ppp));
  ppp = 107038380838084L;
  ppp = initialScramble(ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
  ppp = 246345201500483L;
  ppp = initialScramble(ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
  ppp = 240144965573432L;
  ppp = initialScramble(ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
    return 0;
}
