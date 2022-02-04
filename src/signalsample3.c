
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile int flag = 0;
static pthread_t calledthread = 0;
static pthread_t mainthread = 0;

/**
 * signal sample
 * https://www.jpcert.or.jp/sc-rules/c-sig30-c.html
 * signalハンドラ内で fprintf は未定義動作
 * signal系のAPI多くねえ？
 * signal(2)
 * sigaction(2)
 * signalfd(2)
 * --
 * sigwaitinfo, sigtimedwait - キューに入れられたシグナルを同期して待つ
 * --
 * select, pselect, FD_CLR, FD_ISSET, FD_SET, FD_ZERO - 同期 I/O の多重化
 * poll, ppoll - ファイルディスクリプターにおけるイベントを待つ
 * epoll(7) - I/O イベント通知機能
 * signalfd - シグナル受け付け用のファイルディスクリプターを生成する
 * timerfd_create, timerfd_settime, timerfd_gettime - ファイルディスクリプター経由で通知するタイマー
 * --
 * TODO: Replace signal(2) with sigaction(2)
 */
static void sigint_action(int sig, siginfo_t *t, void*arg)
{
    ucontext_t *context = (ucontext_t *)arg;
    (void)context;
    // fprintf(stdout, "SIGINT recive(%d)\n", sig);
    flag = sig;
    // 大体の場合メイン関数のスレッドと同じスレッドでシグナルを受け取る？
    // あー、だからpthread関係を使うとデッドロックに陥るのか
    calledthread = pthread_self();
}

static volatile int running = 0;

static void *signal_catcher(void *arg)
{
    int sigc = 0;
    int time = 0;
    while (running)
    {
        sigc = flag;
        if (sigc != 0)
        {
            flag = 0;
            fprintf(stdout,
                    "SIGINT recive(%d), main thread : %lu, this thread : %lu, "
                    "signal handler thread : %lu\n",
                    sigc, mainthread, pthread_self(), calledthread);
        }
        // このusleepもシグナルを受け取ったときに抜ける
        // usleepの代わりにsigtimedwaitでシグナルを待つとか……そこまでしなくていいかな
        time = usleep(250000);
    }
    return NULL;
}

static int set_signal_handler(void)
{
    struct sigaction act = { 0 };
    struct sigaction oldact = { 0 };
    act.sa_sigaction = sigint_action;
    int ret = sigaction(SIGINT, &act, &oldact);
    if (ret != 0)
    {
        printf("Error: sigaction() SIGINT: %s\n", strerror(errno));
        perror(NULL);
        return (EXIT_FAILURE);
    }
    printf("func : %d, %p\n", ret, oldact.sa_handler);
    return (EXIT_SUCCESS);
}

#define ERROR_MSG_BUF_SIZE 1024

int main(int argc, char **argv)
{
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }
    pid_t pid = getpid();
    mainthread = pthread_self();
    printf("pid : %d, main thread id : %lu\n", pid, mainthread);
    int rc = set_signal_handler();
    if (rc != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }
    printf("set signal handler ok\n");
    pthread_t catcherthread = 0;
    pthread_create(&catcherthread, NULL, signal_catcher, NULL);
    // 切り離し
    pthread_detach(catcherthread);
    int ret = 0;
    struct timespec req = { 15, 0 };
    struct timespec rem = { 0 };
    int errno_tmp = 0;
    char errmsgbuf[ERROR_MSG_BUF_SIZE];
    int retval = 0;
    do
    {
        // シグナルを受け取ると抜ける
        ret = nanosleep(&req, &rem);
        if (ret != 0)
        {
            printf("ret : %d, %2ld.%09ld\n", ret, rem.tv_sec, rem.tv_nsec);
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;
        }
        // errno チェック
        errno_tmp = errno;
        if (errno_tmp != 0)
        {
            errno = 0;
            retval = strerror_r(errno_tmp, errmsgbuf, ERROR_MSG_BUF_SIZE);
            if (retval == 0)
                printf("%s\n", errmsgbuf);
            else
                printf("strerror_r error\n");
        }
    } while (ret != 0);
    running = 1;
    printf("sleep break\n");
    return 0;
}
