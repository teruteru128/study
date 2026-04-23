
#include <stdbool.h>
#include <stdio.h>

_Bool hasThree(int n) {
  if (n < 0)
    n = -n;
  while (n > 0) {
    if (n % 10 == 3)
      return true;
    n /= 10;
  }
  return false;
}

int main(int argc, char *argv[]) {
  int A = 298266;
  int B = 300000;
  size_t count = 0;
  for (int i = A; i <= B; i++) {
    if (i % 3 == 0 || hasThree(i)) {
      count++;
    }
  }
  printf("A=%d から B=%d の間でアホになる数は %zu 個です。\n", A, B, count);
  printf("%lf\n", (double)count / (B - A));

  return 0;
}
