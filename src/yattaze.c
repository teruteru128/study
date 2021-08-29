
#include "yattaze.h"
#include <stdio.h>
#include <string.h>

void printYattaze(void) { fputs(YATTAZE, stdout); }

int main(void)
{
    // printYattaze();
    size_t len = strlen(YATTAZE);
    fputs("\"", stdout);
    for (size_t i = 0; i < len; i++)
    {
        if (YATTAZE[i] == '\n')
        {
            fputs("\\n\"\\ \n\"", stdout);
        }
        else
        {
            fprintf(stdout, "\\x%02x", YATTAZE[i] & 0xff);
        }
    }
    fputs("\"", stdout);
    return 0;
}
