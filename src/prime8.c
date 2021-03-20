
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

struct task;
struct task
{
    unsigned int offset;
    int answer;
    int tid;
    int padding;
    size_t index;
    time_t timediff;
    struct task *next;
};

#define QUEUE_SIZE 1048576UL

static struct task *area_head = NULL;

/**
 * @brief 未使用領域を並べた連結リスト
 * 
 */
static struct task *unused_area_list_head = NULL;
static struct task *unused_area_list_tail = NULL;
static pthread_mutex_t unused_area_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t unused_area_list_cond = PTHREAD_COND_INITIALIZER;

/**
 * @brief タスクキュー
 * タスクキュー
 * 完了済みタスクキュー
 * 
 * キュー1項目の内容
 * . int 素数候補のbaseからのoffset
 * 
 */
struct task *task_queue_head = NULL;
struct task *task_queue_tail = NULL;
static pthread_mutex_t task_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t task_queue_cond = PTHREAD_COND_INITIALIZER;
static size_t q_head_index = 0;

/**
 * @brief 完了済みタスクリスト
 *
 * キュー1項目の内容
 * . int 素数候補のbaseからのoffset
 * . int 判定結果
 * 
 */
static struct task *completed_task_queue_head = NULL;
static struct task *completed_task_queue_tail = NULL;
static size_t ct_head_index = 0;
static size_t ct_tail_index = 0;
static size_t ct_tail = 0;
static size_t ct_num = 0;
static pthread_mutex_t completed_task_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t completed_task_queue_cond = PTHREAD_COND_INITIALIZER;

/**
 * @brief 
 * 
 * @return struct task* 
 */
struct task *unused_area_pop()
{
    pthread_mutex_lock(&unused_area_list_mutex);
    if (unused_area_list_head == NULL)
    {
        pthread_cond_wait(&unused_area_list_cond, &unused_area_list_mutex);
    }
    struct task *area = unused_area_list_head;
    unused_area_list_head = area->next;
    pthread_mutex_unlock(&unused_area_list_mutex);
    return area;
}

/**
 * @brief 
 * 
 * @param task 
 */
void unused_area_push(struct task *task)
{
    pthread_mutex_lock(&unused_area_list_mutex);
    if (unused_area_list_head == NULL)
    {
        unused_area_list_head = task;
    }
    if (unused_area_list_tail != NULL)
    {
        unused_area_list_tail->next = task;
    }
    unused_area_list_tail = task;
    pthread_mutex_unlock(&unused_area_list_mutex);
}

/**
 * @brief 
 * 
 * @param task 
 * @return int 
 */
int task_enqueue(struct task *task)
{
    pthread_mutex_lock(&task_queue_mutex);
    if (task_queue_head == NULL)
    {
        task_queue_head = task;
    }
    if (task_queue_tail != NULL)
    {
        task_queue_tail->next = task;
    }
    task_queue_tail = task;
    pthread_mutex_unlock(&task_queue_mutex);
    return 0;
}

/**
 * @brief 
 * 
 * @param offset 
 * @return int 
 */
int add_task(unsigned int offset)
{
    // 未使用領域をpop
    struct task *task = unused_area_pop();
    // 書き込み
    task->offset = offset;
    task->answer = 0;
    task->tid = 0;
    task->padding = 0;
    task->index = 0;
    task->timediff = 0;
    task->next = NULL;
    //タスクキューに追加
    task_enqueue(task);
}

/**
 * @brief 
 * 
 * @return struct task* 
 */
struct task *task_dequeue()
{
    pthread_mutex_lock(&task_queue_mutex);
    if (task_queue_head == NULL)
    {
        pthread_cond_wait(&task_queue_cond, &task_queue_mutex);
    }
    struct task *task = task_queue_head;
    task_queue_head = task->next;
    pthread_mutex_unlock(&task_queue_mutex);
    return task;
}

/**
 * @brief 
 * 
 * @param offset 
 * @return int 
 */
unsigned int pop_task()
{
    struct task *task = task_dequeue();

    unsigned int offset = task->offset;
    task->offset = 0;

    unused_area_push(task);
    return offset;
}

int completed_task_enqueue(struct task *task)
{
    pthread_mutex_lock(&completed_task_queue_mutex);
    if (completed_task_queue_head == NULL)
    {
        completed_task_queue_head = task;
    }
    if (completed_task_queue_tail != NULL)
    {
        completed_task_queue_tail->next = task;
    }
    completed_task_queue_tail = task;
    pthread_cond_signal(&completed_task_queue_cond);
    pthread_mutex_unlock(&completed_task_queue_mutex);
}

/**
 * @brief 完了済みタスクキューにタスクを追加します.
 * 
 * この関数は追加が完了するまでブロックします。
 * 
 * @param data 
 * @param offset 
 * @param answer 
 * @param index 
 * @param tid 
 * @param timediff 
 * @return int 
 */
int add_completed_task(unsigned int offset, int answer, size_t index, const int tid, time_t timediff)
{
    struct task *task = unused_area_pop();
    task->offset = offset;
    task->answer = answer;
    task->tid = tid;
    task->padding = 0;
    task->index = index;
    task->timediff = timediff;
    completed_task_enqueue(task);
    return 0;
}

/**
 * @brief 
 * 
 * @param data 
 * @return int 
 */
struct task *completed_task_dequeue()
{
    pthread_mutex_lock(&completed_task_queue_mutex);
    if (completed_task_queue_head == NULL)
    {
        pthread_cond_wait(&completed_task_queue_cond, &completed_task_queue_mutex);
    }
    struct task *task = completed_task_queue_head;
    completed_task_queue_head = completed_task_queue_head->next;
    task->next = NULL;
    pthread_mutex_unlock(&completed_task_queue_mutex);
    return task;
}

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
    unsigned int index = 0;
    unsigned int offset = 1;
    //size_t max = searchSieve.length <= 1000? searchSieve.bits_length : 1000;
    pthread_mutex_lock(&task_queue_mutex);

    size_t q_tail_index = 0;
    for (size_t i = 0; i < searchSieve.bits_length; i++)
    {
        unsigned long nextLong = ~searchSieve.bits[i];
        for (size_t j = 0; j < 64; j++)
        {
            if ((nextLong & 1UL) == 1UL)
            {
                add_task(offset);
                if (offset <= 5559)
                {
                    q_tail_index++;
                    pop_task();
                }
            }
            nextLong >>= 1;
            offset += 2;
        }
    }
    q_head_index = index;
    printf("initial q_tail_index is %zu\n", q_tail_index);
    pthread_cond_broadcast(&task_queue_cond);
    pthread_mutex_unlock(&task_queue_mutex);

#if 0
    while(threadpool_live)
    {
        pthread_mutex_lock(&task_queue_mutex);
        /*ここにタスクが減ってきたときのコードを入れる*/
        pthread_mutex_unlock(&task_queue_mutex);
    }
#endif
    bs_free(&searchSieve);
    return NULL;
}

#if 0
/**
 * @brief 前方宣言、定義は下
 */
int task_receiving_thread_live;
#endif

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
    unsigned int offset = 0;
    size_t index = 0;
    int answer = 0;
    time_t start;
    time_t finish;

    while (threadpool_live)
    {
        offset = pop_task();

        mpz_add_ui(candidate, base, offset);
        start = time(NULL);
        answer = mpz_probab_prime_p(candidate, DEFAULT_CERTAINTY);
        finish = time(NULL);

        add_completed_task(offset, answer, index, tid, (time_t)difftime(finish, start));

        /*
        if (answer == 1 || answer == 2)
        {
            threadpool_live = 0;
        }
        */
    }
    mpz_clear(candidate);
    return NULL;
}

#define CONSUMER_THREAD_NUM 12

void init_queue(const size_t queue_size)
{
    struct task *area = calloc(queue_size, sizeof(struct task));
    if (area == NULL)
        exit(EXIT_FAILURE);

    area_head = unused_area_list_head = area;
    for (size_t i = 1; i < queue_size; i++)
    {
        area[i - 1].next = &area[i];
    }
}

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

    unsigned int offset = 0;
    int answer = 0;
    size_t index = 0;
    int tid = 0;
    time_t timediff = 0;
    // TODO: キューからデキューする処理とprintfを分離する
    while (threadpool_live)
    {
        struct task *task = completed_task_dequeue();
        printf("(%2d) %6ld %6zu, %+7d:%d\n", task->tid, task->timediff, task->index, task->offset, task->answer);
        task->offset = 0;
        task->answer = 0;
        task->tid = 0;
        task->padding = 0;
        task->index = 0;
        task->timediff = 0;
        task->next = NULL;
        unused_area_push(task);
    }
    for (int i = 0; i < CONSUMER_THREAD_NUM; i++)
    {
        pthread_join(consumer_threads[i], NULL);
    }
    mpz_clear(base);
    free(area_head);

    return EXIT_SUCCESS;
}
