#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

void printhelp(const char *prg)
{
        fprintf(stderr, "%s <tweet id> <count> [--target=<target count>] [--verbose]\n", prg);
        fprintf(stderr, "世界のナベアツbot カウント30万時刻推定プログラム\n");
        fprintf(stderr, "ツイートIDとカウント番号から30万台に突入する時刻を推定します\n");
}

/**
 * 世界のナベアツbot カウント30万時刻推定プログラム
 * ツイートIDとカウント番号から30万台に突入する時刻を推定します
 * */
int main(int argc, char const *argv[], char const *envp[])
{
    if(argc == 2 && strcmp(argv[1], "--help") == 0)
    {
        printhelp(argv[0]);
        return 0;
    }
    else if (argc < 3)
    {
        printhelp(argv[0]);
        return 1;
    }
    int64_t id = (int64_t)strtol(argv[1], NULL, 10);
    int64_t timewithmilli = (id >> 22) + 1288834974657L;
    int verbose = 0;
    size_t target = 300000ULL;
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "--verbose") == 0)
        {
            verbose = 1;
        }
        else if(strncmp(argv[i], "--target=", strlen("--target=")) == 0)
        {
            target = strtol(argv[i] + strlen("--target="), NULL, 10);
        }
        else if(strncmp(argv[i], "--target", strlen("--target")) == 0)
        {
            if(i + 1 >= argc)
            {
                fprintf(stderr, "a!\n");
                return 1;
            }
            target = strtol(argv[i + 1], NULL, 10);
            i++;
        }
    }
    struct timespec t;
    t.tv_sec = timewithmilli / 1000;
    t.tv_nsec = (timewithmilli % 1000) * 1000000L;
    size_t count = (size_t)strtol(argv[2], NULL, 10);
    time_t goCrazy = t.tv_sec + (target - count) * 5400;
    struct tm tm;
    localtime_r(&goCrazy, &tm);
    char buffer[32];
    strftime(buffer, 32, "%FT%T%z", &tm);
    printf("%s\n", buffer);
    if(verbose)
    {
        int64_t machine = (id >> 12) & 0x3ff;
        int64_t sequence = id & 0xfff;
        printf("machine id: %" PRId64 ", sequence id: %" PRId64 "\n", machine, sequence);
    }
    return 0;
}
