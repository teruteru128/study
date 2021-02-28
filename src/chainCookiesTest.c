
#include <stdio.h>

void chainCookiesTest()
{
  long chain = 30;
  double cookiesPs = 5.587e+26;
  double cookies = 5.877e+32;
  printf("%f\n", chainCookies(chain, cookiesPs * 7, cookies));
  printf("%f = 10^%f\n", 15., log10(15));
}
