
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>

#define A0 1e0L
#define A1 1.57885731355862L
#define P 3e0L

/* https://twitter.com/teruteru128/status/1108266625352892416 */
int main(int argc, char *argv[])
{
  long double a = 0;
  long double an2 = A0;
  long double an = A1;

  printf("a_0 = %Lf\n", A0);
  printf("a_1 = %.52Lf\n", A1);

  int i = 0;
  for (i = 0; i < 100; i++)
  {
    a = (an2 * an2 * an2 + an) / P;
    if (a >= 1000000)
    {
      printf("over!\n");
      break;
    }
    else
    {
      printf("a_%d = %.52Lf\n", i + 2, a);
    }
    an2 = an;
    an = a;
  }
  return EXIT_SUCCESS;
}
