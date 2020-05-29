
#include <stdio.h>

int main(int argc, char *argv[])
{
  int x = 0;
  int y = 0;
  int m = 0;
  for (x = 10; x < 100; x++)
  {
    for (y = 10; y <= x; y++)
    {
      m = x * y;
      if (m % 100 == 63)
      {
        printf("%d = %d * %d\n", m, x, y);
      }
    }
  }
  return 0;
}
