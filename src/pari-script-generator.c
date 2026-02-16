
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    return 1;
  }
  FILE *fin;
  int append = 0;
  size_t counter = 1;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--append") == 0) {
      append = 1;
    } else if(strcmp(argv[i], "--counter") == 0 && i + i < argc) {
        counter = strtoull(argv[i + 1], NULL, 10);
        i++;
    } else {
      fin = fopen(argv[i], "r");
      if (!fin) {
          perror("fopen r");
        return 1;
      }
    }
  }
  FILE *tests = fopen("gp-tests.gp", append ? "a" : "w");
  if (!tests) {
    fclose(fin);
    perror("fopen a");
    return 1;
  }
  if (!append) {
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
