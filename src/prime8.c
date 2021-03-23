
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

#include <gmp.h>

#define BIT_LENGTH 524288
#define SEARCH_LENGTH (BIT_LENGTH / 20 * 64)
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
    struct BitSieve searchSieve;
    bs_initInstance(&searchSieve, &base, (size_t)SEARCH_LENGTH);
    unsigned int offset = 1;
    //size_t max = searchSieve.length <= 1000? searchSieve.bits_length : 1000;

    size_t skiped_index = 0;
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
                if (offset <= 0)
                {
                    skiped_index++;
                }
                else
                {
                    // 1回1回ロックしてアンロックしてーってやるの時間の無駄なのでiループの外側でまとめてロックしたい
                    // でもデッドロックの危険性とか出るんだろうな
                    add_task(offset, task_index);
                }
                task_index++;
            }
            nextLong >>= 1;
            offset += 2;
        }
    }
    printf("initial q_tail_index is %zu\n", skiped_index);

#if 0
    while(threadpool_live)
    {
        pthread_mutex_lock(&task_queue_mutex);
        /*
        ここに残りタスクが減ってきたときのコードを入れる
        残りタスク数を別変数で持ちたいけど持ったら連結キューにした意味がなくなるんだよなぁ……
        完了済みタスクキューに追加したシグナルで未着手タスクキューが空かどうかチェックすればいいのでは！？
        完了済みタスクキューのシグナルを受け取って未着手タスクキューの状況を確認するってどのmutex取ればいいんですかね？
        */
        pthread_mutex_unlock(&task_queue_mutex);
    }
#endif
    bs_free(&searchSieve);
    return NULL;
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
    time_t start = 0;
    time_t finish = 0;

    while (threadpool_live)
    {
        task = task_dequeue();
        offset = task->offset;
        index = task->index;
        unused_area_enqueue(task);

        mpz_add_ui(candidate, base, offset);
        printf("(%2d)      0, %6zu, %7d:start\n", tid, index, offset);
        start = time(NULL);
        answer = mpz_probab_prime_p(candidate, DEFAULT_CERTAINTY);
        finish = time(NULL);

        // add_completed_task(offset, answer, index, tid, (time_t)difftime(finish, start));

        printf("(%2d) %6ld, %6zu, %7d:%d\n", tid, (time_t)difftime(finish, start), index, offset, answer);
        if (answer == 1 || answer == 2)
        {
            threadpool_live = 0;
        }
    }
    mpz_clear(candidate);
    return NULL;
}

#define CONSUMER_THREAD_NUM 12

/**
 * @brief 完了済みタスクキューのゴミ掃除係
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    mpz_init(base);
    init_queue(QUEUE_SIZE);
    FILE *fin = fopen("524288bit-7d7a92f9-0a35-4cb2-bc1f-fd0e43486e61-initialValue.txt", "r");
    if (fin == NULL)
    {
        mpz_clear(base);
        perror("fopen");
        return EXIT_FAILURE;
    }
    mpz_inp_str(base, fin, 16);
    fclose(fin);
    fin = NULL;

    pthread_t producer_thread;
    pthread_t consumer_threads[CONSUMER_THREAD_NUM];
    int tids[CONSUMER_THREAD_NUM];

    pthread_create(&producer_thread, NULL, produce_prime_candidate, NULL);
    for (int i = 0; i < CONSUMER_THREAD_NUM; i++)
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
    for (int i = 0; i < CONSUMER_THREAD_NUM; i++)
    {
        pthread_join(consumer_threads[i], NULL);
    }
    mpz_clear(base);

    free_queue();

    return EXIT_SUCCESS;
}
