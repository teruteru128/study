
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static void call(void)
{
    size_t count = 30;
    size_t i = 0;
    for (; i < count; i++)
    {
        printf("call-%c\n", 'A' + (char)i % 26);
        sleep(1);
    }
    printf("call-%c\n", 'A' + (char)i % 26);
}

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

// pthread_onceは同期呼び出しなのか非同期予備なしなのかのテスト
int main(int argc, char const *argv[])
{
    fputs("a\n", stdout);
    // 同期呼び出し
    pthread_once(&once_control, call);
    fputs("d\n", stdout);
    return 0;
}
