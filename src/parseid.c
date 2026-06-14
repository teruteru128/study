
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

int64_t parse(int64_t id)
{
    int64_t timewithmilli = (id >> 22) + 1288834974657L;
    struct tm tm;
    time_t t = timewithmilli / 1000;
    localtime_r(&t, &tm);
    char buffer[32];
    strftime(buffer, 32, "%FT%T", &tm);
    printf("%s\n", buffer);
    return 0;
}

int main(int argc, char *argv[], char *envp[])
{
    char *text = "1979146015413318096";
    int64_t id = strtoll(text, NULL, 10);
    parse(id);
    return EXIT_SUCCESS;
}
