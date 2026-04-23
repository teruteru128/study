#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

void printhelp(const char *prg)
{
        fprintf(stderr, "%s [--verbose] [--target=<target count>] <tweet id> <count>\n", prg);
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

    int verbose = 0;
    size_t target = 300000ULL;
    int positional_count = 0;
    int64_t id = 0;
    size_t count = 0;

    // フラグと位置引数を処理
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "--verbose") == 0 || strcmp(argv[1], "-v") == 0)
        {
            verbose = 1;
        }
        else if(strncmp(argv[i], "--target=", strlen("--target=")) == 0)
        {
            target = strtol(argv[i] + strlen("--target="), NULL, 10);
        }
        else if(strcmp(argv[i], "--target") == 0)
        {
            if(i + 1 >= argc)
            {
                fprintf(stderr, "引数が不足しています!\n");
                return 1;
            }
            target = strtol(argv[i + 1], NULL, 10);
            i++;
        }
        else if(argv[i][0] != '-')  // フラグではない = 位置引数
        {
            if(positional_count == 0)
            {
                id = (int64_t)strtol(argv[i], NULL, 10);
            }
            else if(positional_count == 1)
            {
                count = (size_t)strtol(argv[i], NULL, 10);
            }
            positional_count++;
        }
    }
    
    // tweet id と count の両方が指定されたか確認
    if(positional_count < 2)
    {
        printhelp(argv[0]);
        return 1;
    }
    
    int64_t timewithmilli = (id >> 22) + 1288834974657L;
    struct timespec t;
    t.tv_sec = timewithmilli / 1000;
    t.tv_nsec = (timewithmilli % 1000) * 1000000L;
    ssize_t diff = target - count;
    time_t goCrazy = t.tv_sec + diff * 5400;
    struct tm tm;
    localtime_r(&goCrazy, &tm);
    char buffer[32];
    strftime(buffer, 32, "%FT%T%z", &tm);
    printf("到達時刻: %s\n", buffer);
    if(verbose)
    {
        struct tm tm2;
        localtime_r(&t.tv_sec, &tm2);
        char buffer2[32];
        strftime(buffer2, 32, "%FT%T%z", &tm2);
        printf("ポスト時刻: %s\n", buffer2);
        printf("残りポスト数: %zd\n", diff);
        int64_t machine = (id >> 12) & 0x3ff;
        int64_t sequence = id & 0xfff;
        printf("machine id: %" PRId64 ", sequence id: %" PRId64 "\n", machine, sequence);
    }
    return 0;
}
