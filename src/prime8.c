
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "bitsieve.h"

#include <gmp.h>

#define BIT_LENGTH 262144
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

struct task;
struct task
{
    unsigned int offset;
    int answer;
    size_t index;
    struct task *next;
};

#define QUEUE_SIZE 1048576
/**
 * @brief タスクキュー
 * 
 * キュー1項目の内容
 * . int 素数候補のbaseからのoffset
 * 
 */
struct task *task_queue;
//struct task *task_queue_tail;
pthread_mutex_t task_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t task_queue_cond = PTHREAD_COND_INITIALIZER;
unsigned int q[QUEUE_SIZE] = {0};
size_t q_head_index = 0;
size_t q_tail_index = 2121;
#if 0
size_t q_num = 0;
size_t q_tail = 0;
#endif

/**
 * @brief 完了済みタスクリスト
 *
 * キュー1項目の内容
 * . int 素数候補のbaseからのoffset
 * . int 判定結果
 * 
 */
struct task completed_task_queue[QUEUE_SIZE];
size_t ct_head_index = 0;
size_t ct_tail_index = 0;
#if 0
size_t ct_tail = 0;
size_t ct_num = 0;
#endif
//struct task *completed_task_queue_tail;
pthread_mutex_t completed_task_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t completed_task_queue_cond = PTHREAD_COND_INITIALIZER;

#if 0
int task_enqueue(unsigned int offset)
{
    if (q_num < QUEUE_SIZE)
    {
        q[(q_tail + q_num) & QUEUE_SIZE] = offset;
        q_num;
        return 0;
    }
    else
    {
        return 1;
    }
}
int task_dequeue(unsigned int *offset)
{
    if (q_num > 0)
    {
        *offset = q[q_tail];
        q_tail = (q_tail + 1) % QUEUE_SIZE;
        q_num--;
        return 1;
    }
    else
    {
        return 1;
    }
}
int completed_task_enqueue(struct task *data)
{
    if (ct_num < QUEUE_SIZE)
    {
        completed_task_queue[(ct_tail + ct_num) & QUEUE_SIZE] = *data;
        ct_num++;
        return 0;
    }
    else
    {
        return 1;
    }
}
int completed_task_dequeue(struct task *data)
{
    if (ct_num > 0)
    {
        *data = completed_task_queue[ct_tail];
        ct_tail = (ct_tail + 1) % QUEUE_SIZE;
        ct_num--;
        return 1;
    }
    else
    {
        return 1;
    }
}
#endif

/**
 * TODO: もうちょっとまともにキューを実装する
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
    unsigned int index = 0;
    pthread_mutex_lock(&task_queue_mutex);
    for (size_t i = 0; i < searchSieve.bits_length; i++)
    {
        unsigned long nextLong = ~searchSieve.bits[i];
        for (size_t j = 0; j < 64; j++)
        {
            if ((nextLong & 1) == 1)
            {
                q[index++] = (i * 64 + j) * 2 + 1;
            }
            nextLong >>= 1;
        }
    }
    q_head_index = index;
    pthread_cond_broadcast(&task_queue_cond);
    pthread_mutex_unlock(&task_queue_mutex);
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
    mpz_t candidate;
    mpz_init(candidate);
    unsigned int offset = 0;
    size_t index = 0;
    while (threadpool_live)
    {
        pthread_mutex_lock(&task_queue_mutex);
        if (q_tail_index >= q_head_index)
        {
            pthread_cond_wait(&task_queue_cond, &task_queue_mutex);
        }
        index = q_tail_index++;
        offset = q[index];
        pthread_mutex_unlock(&task_queue_mutex);

        mpz_add_ui(candidate, base, offset);
        int answer = mpz_probab_prime_p(candidate, DEFAULT_CERTAINTY);

        pthread_mutex_lock(&completed_task_queue_mutex);
        completed_task_queue[ct_head_index].offset = offset;
        completed_task_queue[ct_head_index].answer = answer;
        completed_task_queue[ct_head_index].index = index;
        completed_task_queue[ct_head_index].next = NULL;
        ct_head_index++;
        pthread_cond_signal(&completed_task_queue_cond);
        pthread_mutex_unlock(&completed_task_queue_mutex);

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
    FILE *fin = fopen("262144bit-initialValue2.txt", "r");
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

    pthread_create(&producer_thread, NULL, produce_prime_candidate, NULL);
    for (int i = 0; i < CONSUMER_THREAD_NUM; i++)
    {
        pthread_create(&consumer_threads[i], NULL, consume_prime_candidate, NULL);
    }
    pthread_join(producer_thread, NULL);

    unsigned int offset = 0;
    int answer = 0;
    size_t index = 0;
    while (threadpool_live)
    {
        pthread_mutex_lock(&completed_task_queue_mutex);
        if (ct_tail_index >= ct_head_index)
        {
            pthread_cond_wait(&completed_task_queue_cond, &completed_task_queue_mutex);
        }
        while (ct_tail_index < ct_head_index)
        {
            offset = completed_task_queue[ct_tail_index].offset;
            answer = completed_task_queue[ct_tail_index].answer;
            index = completed_task_queue[ct_tail_index].index;
            //if (answer == 1 || answer == 2)
            {
                printf("%zu +%d:%d\n", index, offset, answer);
            }
            completed_task_queue[ct_tail_index].offset = 0;
            completed_task_queue[ct_tail_index].answer = 0;
            completed_task_queue[ct_tail_index].index = 0;
            ct_tail_index++;
        }
        pthread_mutex_unlock(&completed_task_queue_mutex);
    }
    for (int i = 0; i < CONSUMER_THREAD_NUM; i++)
    {
        pthread_join(consumer_threads[i], NULL);
    }
    mpz_clear(base);

    return EXIT_SUCCESS;
}
