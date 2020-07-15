
#include <stdio.h>
#include <stdint.h>

struct pair_t
{
  uint64_t p;
  uint64_t q;
};

int main(int argc, char *argv[])
{
  struct pair_t pair[] = {
{421L , 303L}, // 127563
{5519L , 5477L}, // 30227563
{2549L , 4287L}, // 10927563
{551L , 413L}, // 227563
{6647L , 8429L}, // 56027563
{7647L , 1429L}, // 10927563
{5653L , 5471L}, // 30927563
{1783L , 3661L}, // 6527563
{4783L , 2661L}, // 12727563
{7783L , 1661L}, // 12927563
{7901L , 9863L}, // 77927563
{2993L , 6491L}, // 19427563
{6997L , 8479L}, // 59327563
{7997L , 1479L}, // 11827563
{0L, 0L}
};
  size_t i = 0;
  uint64_t x = 0;
  uint64_t y = 0;
  uint64_t m = 0;
  printf("{\n");
  for (i = 0; pair[i].p != 0; i++)
  {
    for (x = 0; x < 10; x++)
    {
      for (y = 0; y < 10; y++)
      {
        if ((m = (x * 10000 + (pair + i)->p) * (y * 10000 + (pair + i)->q)) % 10000000 == 27563)
        {
          printf("{%2$ldL , %3$ldL}, // %1$ld\n", m, x * 10000 + (pair + i)->p, y * 10000 + (pair + i)->q);
        }
      }
    }
  }
  printf("{0L, 0L}\n}\n");
  return 0;
}
