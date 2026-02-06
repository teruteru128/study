
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[])
{
    if(argc < 2)
    {
        return 1;
    }
    char buffer[BUFSIZ];
    FILE *in = fopen(argv[1], "r");
    FILE *out = NULL;
    size_t currentDigits = 0;
    size_t prevDigits = 0;
    char name[NAME_MAX];
    while(fgets(buffer, BUFSIZ, in))
    {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        currentDigits = strlen(buffer);
        if(currentDigits != prevDigits)
        {
            if(out != NULL)
            {
                fclose(out);
                out = NULL;
            }
            snprintf(name, NAME_MAX, "factordb_export_%zudigits.txt", currentDigits);
            fprintf(stderr, "opening %s...\n", name);
            out = fopen(name, "a");
        }
        fprintf(out, "%s\n", buffer);
        prevDigits = currentDigits;
    }
    if(out != NULL)
    {
        fclose(out);
        out = NULL;
    }
    return 0;
}

