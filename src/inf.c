
#include <locale.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]) {
  if (!setlocale(LC_ALL, "")) {
    return 1;
  }
  printf("\U0000221E\n");
  int codepoint;
  for (codepoint = 0x2200; codepoint < 0x2300; codepoint++) {
    if ((codepoint + 1) % 16 == 1) {
      printf("[");
    }
    printf("%lc", codepoint);
    if ((codepoint + 1) % 16 == 0) {
      printf("]\n");
    }
  }
  return 0;
}
