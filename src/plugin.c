
#include <dlfcn.h>
#include <malloc.h>
#include <plugin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>

void *aaaa_aaaa(void *message) {
  printf("[Optional Feature] Plugin running: %s\n", (char *)message);
  void *handle1 = dlopen("libm.so.6", RTLD_LAZY);
  void *handle2 = dlopen("m", RTLD_LAZY);
  printf("libm.so: %p, m: %p\n", handle1, handle2);
  printf("%s\n", handle1 == handle2 ? "true" : "false");
end:
  if (handle1)
    dlclose(handle1);
  if (handle2)
    dlclose(handle2);
  return NULL;
}

void *time32a(void *arg) {
  time_t end = (1L << 31) - 1;
  time_t now;
  int64_t total_seconds = 0;
  while ((total_seconds = (int64_t)difftime(end, now = time(NULL))) > 0) {
    // 2. すべて独立した変数として計算（上書きしない）
    const int64_t hours    = total_seconds / 3600;
    const int64_t minutes = (total_seconds % 3600) / 60;
    const int64_t seconds = total_seconds % 60;
    printf("%"PRId64 " hours %"PRId64" minutes %"PRId64" seconds\n", hours, minutes, seconds);
    sleep(1);
  }
  return NULL;
}

void *time33a(void *arg) {
    time_t end = (1L << 31) - 1;
    time_t target = end - 100000 * 3600;
    struct tm local_tm;
    localtime_r(&target, &local_tm);
    // 3. strftime を使ってフォーマット指定（例: YYYY-MM-DD HH:MM:SS）
    char buf[64];
    if (strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &local_tm) == 0) {
        fprintf(stderr, "strftime failed\n");
        return NULL;
    }
    printf("%s\n", buf);
    return NULL;
}
