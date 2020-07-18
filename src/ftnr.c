
#include <stdio.h>
#include <stdlib.h>
#include <java_random.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

/*
 * 9.3cm, 6.243646ml
 * 10.0cm, 47081.586164ml
 * 11.9cm, 304.445320ml
 * 18.2cm, 8.055883ml
 * 20.3cm, 4.031057ml
 * 21.1cm, 4608.490765ml
 * 25.2cm, 582.253892ml
 * 27.7cm, 2.804713ml
 * 28.2cm, 10962.108695ml
 * 29.7cm, 27210.034929ml
 * 31.8cm, 2037.117076ml
 * 32.9cm, 5.508962ml
 * 36.5cm, 1698.530900ml
 * 38.2cm, 74641.376689ml
 * 38.4cm, 167.290089ml
 * 39.4cm, 3465.697319ml
 *
 * 27.7cm, 2.804713ml
 * 20.3cm, 4.031057ml
 * 32.9cm, 5.508962ml
 * 9.3cm, 6.243646ml
 * 18.2cm, 8.055883ml
 * 38.4cm, 167.290089ml
 * 11.9cm, 304.445320ml
 * 25.2cm, 582.253892ml
 * 36.5cm, 1698.530900ml
 * 31.8cm, 2037.117076ml
 * 39.4cm, 3465.697319ml
 * 21.1cm, 4608.490765ml
 * 28.2cm, 10962.108695ml
 * 29.7cm, 27210.034929ml
 * 10.0cm, 47081.586164ml
 * 38.2cm, 74641.376689ml
 */
int main(int argc, char const *argv[])
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seed = (tv.tv_usec & 0x000FFFFF) << 28;
  seed ^= tv.tv_sec;
  int64_t rnd = initialScramble(seed);
  int i = 0;
  for (; i < 5; i++)
    printf("%.1fcm, %lfml\n", (nextFloat(&rnd) * 310 + 90) / 10, pow(10, nextFloat(&rnd) * 4));
  return EXIT_SUCCESS;
}
