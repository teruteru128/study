
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * PARI/GP スクリプトファイル生成プログラム
 */
int main(int argc, char const *argv[]) {
  if (argc < 2) {
    return 1;
  }
  FILE *fin;
  int append = 0;
  size_t counter = 1;
  char outfile[PATH_MAX] = "gp-tests.txt";
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--append") == 0) {
      // 追加モード
      append = 1;
    } else if(strcmp(argv[i], "--counter") == 0 && i + 1 < argc) {
        // カウンター
        counter = strtoull(argv[i + 1], NULL, 10);
        i++;
    } else if(strcmp(argv[i], "--outfile") == 0 && i + 1 < argc) {
        // 出力ファイル
        size_t length = strlen(argv[i+1]);
        strncpy(outfile, argv[i +1], length < PATH_MAX? length : PATH_MAX);
        i++;
    } else {
        // 入力ファイル
      fin = fopen(argv[i], "r");
      if (!fin) {
          perror("fopen r");
        return 1;
      }
    }
  }
  FILE *tests = fopen(outfile, append ? "a" : "w");
  if (!tests) {
    fclose(fin);
    perror("fopen a");
    return 1;
  }
  if (!append) {
      // 追加モードの場合設定は書き込まない
    fputs("default(nbthreads, 16);\n", tests);
    fputs("print(\"nbthreads->\", default(nbthreads), \"<-\");\n", tests);
  }
  char line[BUFSIZ];
  while (fgets(line, BUFSIZ, fin)) {
    line[strcspn(line, "\r\n")] = '\0';
    if (line[0] == '\0')
      continue;
    size_t digits = strlen(line);

    fprintf(tests,
            "write(\"certs/%04zu-cert%zu.txt\", primecertexport(primecert(%s), 1))\n",
            counter, digits, line);
    counter++;
  }

  fclose(tests);
  fclose(fin);
  fprintf(stderr, "Succsess: %zu\n", counter);
  return 0;
}
