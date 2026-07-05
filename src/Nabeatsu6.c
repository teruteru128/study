
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/**
* [ナベアツbot](https://x.com/3_aho_bot)で30万までのカウントを毎秒表示
*/
int main(int argc, char *argv[]) {
  size_t count = 0;
  size_t target = 300000;
  int64_t id = 0;
  int positional_count = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--verbose") == 0) {
    } else {
      // 位置引数
      switch (positional_count) {
      case 0:
         id = (int64_t) strtol(argv[i], NULL, 10);
        break;
      case 1:
        count = (size_t)strtoul(argv[i], NULL, 10);
        break;
      default:
        break;
      }
      positional_count++;
    }
  }
  int64_t timewithmilli = (id >> 22) + 1288834974657L;
  struct timespec t;
  t.tv_sec = timewithmilli / 1000;
  t.tv_nsec = (timewithmilli % 1000) * 1000000L;

  ssize_t diff = target - count;
  time_t diffSeconds = diff * 5400;
  // カウント300000の時刻を計算する
  time_t goCrazy = t.tv_sec + diffSeconds;
  time_t current = 0;
  double timediff = 0;
  // 現在時刻から300000までの時間を計算する
  while((timediff = difftime(goCrazy, time(NULL))) > 0 ) {
    // 90分の秒数で割った値を表示する
    printf("\r%.9f counts", timediff / 5400);
    fflush(stdout);
    sleep(1);
  }
  return EXIT_SUCCESS;
}
