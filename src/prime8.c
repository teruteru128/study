
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
#include "bitsieve.h"
#include "task_queue.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <sys/sysinfo.h>

#include <gmp.h>

#define DEFAULT_CERTAINTY 1

struct BitSieve
{
    unsigned long *bits;
    size_t bits_length;
    size_t length;
};

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
    const size_t min_offset = *(size_t *)arg;
    struct BitSieve searchSieve;
    const size_t searchLength = mpz_sizeinbase(base, 2) / 20 * 64;
    bs_initInstance(&searchSieve, &base, searchLength);
    unsigned int offset = 1;
    //size_t max = searchSieve.length <= 1000? searchSieve.bits_length : 1000;

    size_t skiped_num = 0;
    size_t task_index = 0;
    // ここでタスクキューをロックしてまとめて追加したい
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
                    // 1回1回ロックしてアンロックしてーってやるの時間の無駄なのでiループの外側でまとめてロックしたい
                    // でもデッドロックの危険性とか出るんだろうな
                    add_unstarted_task(offset, task_index);
                }
                task_index++;
            }
            nextLong >>= 1;
            offset += 2;
        }
    }
    printf(ngettext("One prime number candidate was found.\n", "%zu prime number candidates were found.\n", task_index), task_index);
    printf(ngettext("One prime number candidate have been skipped.\n", "%zu prime number candidates have been skipped.\n", skiped_num), skiped_num);

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

struct timespec *difftimespec(struct timespec *result, struct timespec *time1, struct timespec *time0)
{
    if ((time1->tv_nsec - time0->tv_nsec) < 0)
    {
        result->tv_sec = time1->tv_sec - time0->tv_sec - 1;
        result->tv_nsec = time1->tv_nsec - time0->tv_nsec + 1000000000;
    }
    else
    {
        result->tv_sec = time1->tv_sec - time0->tv_sec;
        result->tv_nsec = time1->tv_nsec - time0->tv_nsec;
    }
    return result;
}

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
    const int tid = *((int *)arg);
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
        unused_area_enqueue(task);

        mpz_add_ui(candidate, base, offset);
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
        printf("(%2d) %s, %6ld, %6zu, %7d:%d\n", tid, formattedTime, diff.tv_sec, index, offset, answer);
        if (answer == 1 || answer == 2)
        {
            task = unused_area_dequeue();
            task->offset = offset;
            threadpool_live = 0;
        }
    }
    mpz_clear(candidate);
    return task;
}

#define CONSUMER_THREAD_NUM 4

int init_base(char *basefilepath)
{
    mpz_init(base);
    init_queue(QUEUE_SIZE);
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
int searchPrime_main(int argc, char *argv[])
{
    initGettext();
    char *basefile = (argc >= 2) ? argv[1] : NULL;
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

    const size_t threadNum = (argc >= 4) ? (size_t)strtoul(argv[3], NULL, 10) : CONSUMER_THREAD_NUM;

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

    pthread_t producer_thread;
    pthread_t *consumer_threads = calloc(threadNum, sizeof(pthread_t));
    if (consumer_threads == NULL)
    {
        perror("consumer_threads = calloc");
        mpz_clear(base);
        return EXIT_FAILURE;
    }
    int *tids = calloc(threadNum, sizeof(int));
    if (tids == NULL)
    {
        perror("tids = calloc");
        free(consumer_threads);
        mpz_clear(base);
        return EXIT_FAILURE;
    }

    size_t min_offset = 0;
    if (argc >= 3)
    {
        min_offset = (size_t)strtoul(argv[2], NULL, 10);
    }

    pthread_create(&producer_thread, NULL, produce_prime_candidate, &min_offset);
    for (size_t i = 0; i < threadNum; i++)
    {
        tids[i] = (int)i;
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
            printf("prime found! offset:%u\n", task->offset);
            //export_found_prime(task->offset);
            unused_area_enqueue(task);
        }
    }
    mpz_clear(base);
    free(consumer_threads);
    free(tids);

    free_queue();

    return EXIT_SUCCESS;
}

/**
 * @brief スレッド起動
 * 完了済みタスクキューのゴミ掃除係
 * prime8 初期値ファイル <offset>
 * コマンドライン引数のオプションの実装ってどうやるんや？
 * --threadとか-tとか
 * --help
 * --version
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    return searchPrime_main(argc, argv);
}
