
#include <limits.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    return 1;
  }
  FILE *fin;
  int append = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--append") == 0) {
      append = 1;
    } else {
      fin = fopen(argv[i], "r");
      if (!fin) {
        return 1;
      }
    }
  }
  FILE *tests = fopen("gp-tests.gp", append ? "a" : "w");
  if (!tests) {
    fclose(fin);
    return 1;
  }
  if (!append) {
    fputs("default(nbthreads, 16);\n", tests);
    fputs("print(\"nbthreads->\", default(nbthreads), \"<-\");\n", tests);
  }
  char line[BUFSIZ];
  size_t counter = 1;
  while (fgets(line, BUFSIZ, fin)) {
    line[strcspn(line, "\r\n")] = '\0';
    if (line[0] == '\0')
      continue;
    size_t digits = strlen(line);

    fprintf(tests,
            "write(\"%04zu-cert%zu.txt\", primecertexport(primecert(%s), 1))\n",
            counter, digits, line);
    counter++;
  }

  fclose(tests);
  fclose(fin);
  return 0;
}
