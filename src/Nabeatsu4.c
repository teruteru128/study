#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

/**
 * ツイートIDとカウント番号から30万台に突入する時刻を推定します
 * */
int main(int argc, char const *argv[], char const *envp[])
{
    if (argc < 3)
    {
        fprintf(stderr, "%s <tweet id> <count> [target count]\n", argv[0]);
        fprintf(stderr, "ツイートIDとカウント番号から30万台に突入する時刻を推定します\n");
        return 1;
    }
    int64_t id = (int64_t)strtol(argv[1], NULL, 10);
    int64_t timewithmilli = (id >> 22) + 1288834974657L;
    int64_t machine = (id >> 12) & 0x3ff;
    int64_t sequence = id & 0xfff;
    struct timespec t;
    t.tv_sec = timewithmilli / 1000;
    t.tv_nsec = (timewithmilli % 1000) * 1000000L;
    size_t count = (size_t)strtol(argv[2], NULL, 10);
    size_t target = (size_t)(argc >= 4 ? strtol(argv[3], NULL, 10) : 300000L);
    time_t goCrazy = t.tv_sec + (target - count) * 5400;
    struct tm tm;
    localtime_r(&goCrazy, &tm);
    char buffer[32];
    strftime(buffer, 32, "%FT%T%z", &tm);
    printf("%s\n", buffer);
    printf("machine id: %" PRId64 ", sequence id: %" PRId64 "\n", machine, sequence);
    return 0;
}
