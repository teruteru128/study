
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>

int main(int argc, char *argv[]){
  const double a0 = 1;
  const double a1 = 1.5788573135586235762772844282153529;
  const double p = 3;
  double a = 0;
  double an2 = a0;
  double an = a1;

  printf("%lf\n", a0);
  printf("%lf\n", a1);

  int i=0;
  for(i = 0; i < 100; i ++){
    a = (pow(an2, p) + an) / p;
    if(a >= 1000000){
      printf("over!\n");
      break;
    }else{
      printf("%lf\n", a);
    }
    an2 = an;
    an = a;
  }
  return EXIT_SUCCESS;
}

