
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#define PUBLISH_STRUCT_BS
#include "bitsieve.h"
#include "task_queue.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <ctype.h>
#include <errno.h>
#include "timeutil.h"

#include <gmp.h>

#define DEFAULT_CERTAINTY 1

/**
 * @brief offsetの基準値。offset+baseが素数候補の値になる
 * 
 */
mpz_t base;

/**
 * @brief threadpoolを停止するときは0を代入する
 */
static int threadpool_live = 1;

#define QUEUE_SIZE 1048576UL

struct producer_arg
{
    size_t offset;
    char *path;
};

/**
 * @brief 素数候補を生成してタスクキューに追加する. 終了時のタスクキューのゴミ掃除係
 * 
 * タスク件数はどれくらいの数をキープするべきなんだろうか？
 * . 最初に8万件ぐらい入れて追加はしない
 * . 最初に1000件ぐらい入れてそれ以降は16件未満になったら16件まで追加
 * . 
 * 
 * タスクキューのmutexを取る
 * 初期タスクとして1000件タスクを追加する
 * タスクキューのmutexを開放する
 * 
 * while(threadpool_live)
 * {
 * キューのmutexを取る
 * シグナル待ちしてシグナルを受け取ったらキューの件数を調べる
 * 16未満だったら16まで候補を補充する
 * mutexを開放する
 * }
 * 
 * 素数が見つかったらタスクキューを含めてゴミ掃除する
 * 
 * @param arg 
 * @return void* 
 */
void *produce_prime_candidate(void *arg)
{
    const struct producer_arg *argp = (struct producer_arg *)arg;
    const size_t min_offset = argp->offset;
    char *bs_filename = argp->path;
    struct BitSieve searchSieve;
    const size_t searchLength = mpz_sizeinbase(base, 2) / 20 * 64;
    fprintf(stderr, "search Length is %lu\n", searchLength);
    if (bs_filename != NULL)
    {
        // bsファイルアクセス可能
        FILE *fin = fopen(bs_filename, "rb");
        if (fin == NULL)
        {
            perror("fin in produce_prime_candidate");
            return NULL;
        }
        bs_filein(&searchSieve, fin);
        // fputs("篩を読み込みました。\n", stderr);
        fputs("Bit sieve was loaded.\n", stderr);
        fclose(fin);
    }
    else
    {
        // bsファイルアクセス不可能
        bs_initInstance(&searchSieve, &base, searchLength);
        fputs("Bit sieve was generated.\n", stderr);
    }
    unsigned int offset = 1;
    //size_t max = searchSieve.length <= 1000? searchSieve.bits_length : 1000;

    size_t skiped_num = 0;
    size_t task_index = 0;
    unstarted_task_enqueue_putLock();
    size_t size = unstarted_task_getSize_nolock();
    for (size_t i = 0; i < searchSieve.bits_length; i++)
    {
        unsigned long nextLong = ~searchSieve.bits[i];
        for (size_t j = 0; j < 64; j++)
        {
            // ここの"== 1UL"いらなくない？そのまま"nextLong & 1UL"だけでいいじゃん
            if ((nextLong & 1UL) == 1UL)
            {
                if (offset <= min_offset)
                {
                    skiped_num++;
                }
                else
                {
                    add_unstarted_task_nolock(offset, task_index);
                }
                task_index++;
            }
            nextLong >>= 1;
            offset += 2;
        }
    }
    unstarted_task_enqueue_putUnlock();
    if (size == 0)
        unstarted_task_signal_not_empty();
    fprintf(stderr, ngettext("One prime number candidate was found.\n", "%zu prime number candidates were found.\n", task_index), task_index);
    fprintf(stderr, ngettext("One prime number candidate have been skipped.\n", "%zu prime number candidates have been skipped.\n", skiped_num), skiped_num);

#if 0
    while(threadpool_live)
    {
        pthread_mutex_lock(&unstarted_task_list_mutex);
        /*
        ここに残りタスクが減ってきたときのコードを入れる
        残りタスク数を別変数で持ちたいけど持ったら連結キューにした意味がなくなるんだよなぁ……
        完了済みタスクキューに追加したシグナルで未着手タスクキューが空かどうかチェックすればいいのでは！？
        完了済みタスクキューのシグナルを受け取って未着手タスクキューの状況を確認するってどのmutex取ればいいんですかね？
        */
        pthread_mutex_unlock(&unstarted_task_list_mutex);
    }
#endif
    bs_free(&searchSieve);
    return NULL;
}

#define TIME_FORMAT_BUFFER_SIZE 64

static pthread_barrier_t barrier;

/**
 * @brief 素数候補をタスクキューから取り出して素数判定して完了済みタスクリストに結果を入れる
 * 
 * コンシューマスレッドって何スレッドぐらいよ？12?
 * 
 * タスクキューのmutexを取る→キューの中身が空か調べる
 *   空だったらcondで待つ
 *   シグナルが来たらキューからタスクを取り出す
 * 空でなかったらタスクキューからタスクを取り出す
 * タスクキューのシグナルを送信する
 * タスクキューのmutexを開放する
 * 
 * 候補が素数か判定する
 * 
 * 完了済みタスクリストのmutexを取得してリストに結果を追加する
 * signalを送信する
 * 
 * 素数が見つかったらthreadpool_liveに0を入れて終了させる
 * 
 * @param arg 
 * @return void* 
 */
void *consume_prime_candidate(void *arg)
{
    const size_t tid = *((size_t *)arg);
    mpz_t candidate;
    mpz_init(candidate);
    struct task *task = NULL;
    unsigned int offset = 0;
    size_t index = 0;
    int answer = 0;
    struct timespec start;
    struct timespec finish;
    struct timespec diff;
    struct tm tm = {0};
    char format[TIME_FORMAT_BUFFER_SIZE];
    char formattedTime[TIME_FORMAT_BUFFER_SIZE];

    while (threadpool_live)
    {
        task = unstarted_task_dequeue();
        offset = task->offset;
        index = task->index;
        free(task);
        task = NULL;

        mpz_add_ui(candidate, base, offset);
        pthread_barrier_wait(&barrier);
        // 割と邪魔だな
        //printf("(%2d)      0, %6zu, %7d:start\n", tid, index, offset);
        clock_gettime(CLOCK_REALTIME, &start);
        answer = mpz_probab_prime_p(candidate, DEFAULT_CERTAINTY);
        clock_gettime(CLOCK_REALTIME, &finish);

        // add_completed_task(offset, answer, index, tid, (time_t)difftime(finish, start));

        difftimespec(&diff, &finish, &start);
        localtime_r(&finish.tv_sec, &tm);
        strftime(format, TIME_FORMAT_BUFFER_SIZE, "%FT%T.%%09ld%z", &tm);
        snprintf(formattedTime, TIME_FORMAT_BUFFER_SIZE, format, finish.tv_nsec);
        fprintf(stderr, "(%2lu) %s, %6ld.%09ld, %6zu, %7d:%d\n", tid, formattedTime, diff.tv_sec, diff.tv_nsec, index, offset, answer);
        if (answer == 1 || answer == 2)
        {
            task = calloc(1, sizeof(struct task));
            task->offset = offset;
            threadpool_live = 0;
        }
        pthread_barrier_wait(&barrier);
    }
    mpz_clear(candidate);
    return task;
}

#define CONSUMER_THREAD_NUM 4

int init_base(const char *basefilepath)
{
    mpz_init(base);
    FILE *fin = fopen(basefilepath, "r");
    if (fin == NULL)
    {
        mpz_clear(base);
        perror("fopen");
        return EXIT_FAILURE;
    }
    mpz_inp_str(base, fin, 16);
    fclose(fin);
    fin = NULL;
    return EXIT_SUCCESS;
}

void initGettext()
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
}

/**
 * @brief 
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int searchPrime_main(const int argc, const char *argv[])
{
    initGettext();
    const char *basefile = (argc >= 2) ? argv[1] : NULL;
    if (basefile == NULL)
    {
        fprintf(stderr, "basefileは必須です。\n");
        fprintf(stderr, "%s [初期値ファイル] <offset to skip> <thread num>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (init_base(basefile) != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    size_t tmp = CONSUMER_THREAD_NUM;
    if (argc >= 4)
    {
        errno = 0;
        tmp = (size_t)strtoul(argv[3], NULL, 10);
        if (errno != 0)
        {
            perror("strtoul");
            tmp = CONSUMER_THREAD_NUM;
        }
    }

    const size_t threadNum = tmp;

    if (threadNum == 0)
    {
        fputs("0個のスレッドは指定することが出来ません。\n", stderr);
        // fputs("0個のスレッドは許可されていません。\n", stderr);
        mpz_clear(base);
        return EXIT_FAILURE;
    }
    {
        const long availableProcessors = sysconf(_SC_NPROCESSORS_ONLN);
        if (availableProcessors >= 0 && threadNum > (size_t)availableProcessors)
        {
            fprintf(stderr, "利用可能なプロセッサ数より多くのスレッド数が指定されています。\n");
        }
    }
    pthread_barrier_init(&barrier, NULL, (unsigned int)threadNum);

    pthread_t producer_thread;
    pthread_t *consumer_threads = calloc(threadNum, sizeof(pthread_t));
    if (consumer_threads == NULL)
    {
        perror("consumer_threads = calloc");
        mpz_clear(base);
        return EXIT_FAILURE;
    }
    size_t *tids = calloc(threadNum, sizeof(size_t));
    if (tids == NULL)
    {
        perror("tids = calloc");
        free(consumer_threads);
        mpz_clear(base);
        return EXIT_FAILURE;
    }

    struct producer_arg arg = {0, NULL};
    if (argc >= 3)
    {
        arg.offset = (size_t)strtoul(argv[2], NULL, 10);
    }
    char *bs_pathname = malloc(FILENAME_MAX);
    bs_pathname[0] = 0;
    {
        // 拡張子書き換え
        char *work = strdup(basefile);
        char *dot = strrchr(work, '.');
        if (dot != NULL)
        {
            *dot = '\0';
        }
        if (strlen(work) > FILENAME_MAX)
        {
            fputs("path nameが長すぎます！\n", stderr);
            return EXIT_FAILURE;
        }
        snprintf(bs_pathname, FILENAME_MAX, "%s.bs", work);
        free(work);
    }
    if (access(bs_pathname, F_OK | R_OK) == 0)
    {
        arg.path = bs_pathname;
    }
    else
    {
        free(bs_pathname);
        bs_pathname = NULL;
        arg.path = NULL;
    }

    pthread_create(&producer_thread, NULL, produce_prime_candidate, &arg);
    for (size_t i = 0; i < threadNum; i++)
    {
        tids[i] = i;
        pthread_create(&consumer_threads[i], NULL, consume_prime_candidate, tids + i);
    }
    pthread_join(producer_thread, NULL);

#if 0
    // TODO: キューからデキューする処理とprintfを分離する
    while (threadpool_live)
    {
        struct task *task = completed_task_dequeue();
        if (task->answer == 1 || task->answer == 2)
        {
            threadpool_live = 0;
        }
        printf("(%2d) %6ld, %6zu, %7d:%d\n", task->tid, task->timediff, task->index, task->offset, task->answer);
        unused_area_enqueue(task);
    }
#endif
    struct task *task = NULL;
    for (size_t i = 0; i < threadNum; i++)
    {
        pthread_join(consumer_threads[i], (void **)&task);
        if (task != NULL)
        {
            fprintf(stderr, "prime found! offset:%u\n", task->offset);
            //export_found_prime(task->offset);
            free(task);
        }
    }
    mpz_clear(base);
    pthread_barrier_destroy(&barrier);
    free(consumer_threads);
    free(tids);
    free(bs_pathname);

    return EXIT_SUCCESS;
}

/**
 * @brief エクスポートされたビット篩をインポートして素数探索
 * bitsieveをエクスポートして毎回使い回せば早くなるんじゃねえか？作戦 その2
 * スレッド起動
 * 完了済みタスクキューのゴミ掃除係
 * prime8 初期値ファイル <offset>
 * コマンドライン引数のオプションの実装ってどうやるんや？
 * --threadとか-tとか
 * --help
 * --version
 * 524288bit-7d7a92f9-0a35-4cb2-bc1f-fd0e43486e61-initialValue.txt 78481
 * (11) 2021-04-09T00:52:16.827287200+0900,   3363.858296900,   2332,   78481:0
 * ( 8) 2021-04-09T06:57:20.838852200+0900,   3175.324085300,   2428,   82137:0
 * ( 5) 2021-04-21T12:45:19.708452400+0900,   8349.946415000,   4592,  191319:1
 * 1スレッド, 1800s -> 1時間あたり2タスク, 2倍
 * 8スレッド, 2440s -> 1時間あたり11.8タスク, 1.4倍
 * 16スレッド, 3100s -> 1時間あたり18.6タスク, 1.16倍
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, const char *argv[])
{
    return searchPrime_main(argc, argv);
}
