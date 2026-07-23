
#include <dlfcn.h>
#include <inttypes.h>
#include <malloc.h>
#include <plugin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#define N 3000

void *nextCumshoot(char *msg) {
  srand(time(NULL));

  int num;
  size_t count = 0;
  do {
    num = rand() % N;
    count++;

    if (num == 0) {
      printf("!? "
             "ｳｧｧ!!ｵﾚﾓｲｯﾁｬｳｩｩｩ!!!ｳｳｳｳｳｳｳｳｳｩｩｩｩｩｩｩｩｳｳｳｳｳｳｳｳ!"
             "ｲｨｨｲｨｨｨｲｲｲｨｲｲｲｲｲｲｲｲｲｲｲｲ!!(感度＆射精量3000倍)(%d, %zu)\n",
             num, count);
    } else {
      printf("うっ！ふぅ……(%d, %zu)\n", num, count);
    }
  } while (num != 0);

  return NULL;
}
