
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

static size_t count = 30;
static size_t i = 0;

static void call(void)
{
    for (; i < count; i++)
    {
        printf("call-%d-%c\n", once_control, 'A' + (char)i % 26);
        usleep(250000);
    }
    printf("call-%d-%c\n", once_control, 'A' + (char)i % 26);
    count += 30;
}

// pthread_onceは同期呼び出しなのか非同期予備なしなのかのテスト
int main(int argc, char const *argv[])
{
    fputs("a\n", stdout);
    // 同期呼び出し
    pthread_once(&once_control, call);
    fprintf(stdout, "d%d\n", once_control);
    pthread_once(&once_control, call);
    fprintf(stdout, "e%d\n", once_control);
    return 0;
}
