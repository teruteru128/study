
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
