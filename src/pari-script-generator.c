
#include <stdio.h>
#include <string.h>
#include <limits.h>

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        return 1;
    }
    FILE *fin = fopen(argv[1], "r");
    if (!fin)
    {
        return 1;
    }
    FILE *tests = fopen("gp-tests.gp", "w");
    if (!tests)
    {
        fclose(fin);
        return 1;
    }
    fputs("default(nbthreads, 16);\n", tests);
    fputs("print(\"nbthreads->\", default(nbthreads), \"<-\");", tests);
    char line[BUFSIZ];
    char filename[NAME_MAX];
    size_t counter = 1;
    while (fgets(line, BUFSIZ, fin))
    {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0')
            continue;
        size_t digits = strlen(line);
        snprintf(filename, NAME_MAX, "gp-tests-%zu (%04zu).gp", digits, counter);

        FILE *fout = fopen(filename, "w");
        if (!fout)
        {
            perror("fopen");
            return 1;
        }
        fprintf(fout, "write(\"cert%zu (%04zu).txt\", primecertexport(primecert(%s), 1))\n", digits, counter, line);
        fclose(fout);
        fout = NULL;
        fprintf(tests, "\\r \"%s\"\n", filename);
        counter++;
    }

    fclose(tests);
    fclose(fin);
    return 0;
}
