
#include <stdio.h>
#include <string.h>

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
    char line[BUFSIZ];
    char filename[1024];
    size_t counter = 1;
    while (fgets(line, BUFSIZ, fin))
    {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0')
            continue;
        size_t digits = strlen(line);
        snprintf(filename, 1024, "gp-tests-%zu (%zu).gp", digits, counter);

        FILE *fout = fopen(filename, "w");
        if (!fout)
        {
            perror("fopen");
            return 1;
        }
        fprintf(fout, "write(\"cert%zu (%zu).txt\", primecertexport(primecert(%s), 1))\n", digits, counter, line);
        fclose(fout);
        fout = NULL;
        counter++;
    }

    fclose(fin);
    return 0;
}
