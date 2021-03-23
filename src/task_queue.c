
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "task_queue.h"

/**
 * @brief free用ポインタ
 * 
 */
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
 * 未着手タスクキュー
 * 
 * キュー1項目の内容
 * . int 素数候補のbaseからのoffset
 * 
 */
static struct task *task_queue_head = NULL;
static struct task *task_queue_tail = NULL;
static pthread_mutex_t task_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t task_queue_cond = PTHREAD_COND_INITIALIZER;

/**
 * @brief 完了済みタスクキュー
 * タスク結果キュー
 *
 * キュー1項目の内容
 * . int 素数候補のbaseからのoffset
 * . int 判定結果
 * 
 * completed_taskよりはresultのほうが近い気がする
 */
static struct task *completed_task_queue_head = NULL;
static struct task *completed_task_queue_tail = NULL;
static pthread_mutex_t completed_task_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t completed_task_queue_cond = PTHREAD_COND_INITIALIZER;

/**
 * @brief 
 * 
 * @return struct task* 
 */
struct task *unused_area_dequeue()
{
    pthread_mutex_lock(&unused_area_list_mutex);
    if (unused_area_list_head == NULL)
    {
        pthread_cond_wait(&unused_area_list_cond, &unused_area_list_mutex);
    }

    struct task *area = unused_area_list_head;

    unused_area_list_head = area->next;
    if (area == unused_area_list_tail)
    {
        unused_area_list_tail = NULL;
    }
    area->next = NULL;

    pthread_mutex_unlock(&unused_area_list_mutex);
    return area;
}

/**
 * @brief 
 * 
 * @param task 
 */
void unused_area_enqueue(struct task *task)
{
    if (task == NULL)
    {
        return;
    }
    task->offset = 0;
    task->answer = 0;
    task->tid = 0;
    task->padding = 0;
    task->index = 0;
    task->timediff = 0;
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
    pthread_cond_broadcast(&unused_area_list_cond);
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
    if (task == NULL)
    {
        return 1;
    }
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
    pthread_cond_broadcast(&task_queue_cond);
    pthread_mutex_unlock(&task_queue_mutex);
    return 0;
}

/**
 * @brief 
 * 
 * @param offset 
 * @return int 
 */
int add_task(unsigned int offset, size_t index)
{
    // 未使用領域をpop
    struct task *task = unused_area_dequeue();
    // 書き込み
    task->offset = offset;
    task->answer = 0;
    task->tid = 0;
    task->padding = 0;
    task->index = index;
    task->timediff = 0;
    task->next = NULL;
    //タスクキューに追加
    task_enqueue(task);
    return 0;
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
    if (task == task_queue_tail)
    {
        task_queue_tail = NULL;
    }
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

    unused_area_enqueue(task);
    return offset;
}

/**
 * @brief Returns 1 if the task queue is empty, 0 otherwise.
 * 
 * @return int 
 */
int task_queue_is_empty()
{
    pthread_mutex_lock(&task_queue_mutex);
    int result = task_queue_head == NULL;
    pthread_mutex_unlock(&task_queue_mutex);
    return result;
}

/**
 * @brief 
 * 
 * @param task 
 * @return int 
 */
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
    pthread_cond_broadcast(&completed_task_queue_cond);
    pthread_mutex_unlock(&completed_task_queue_mutex);
    return 0;
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
    struct task *task = unused_area_dequeue();
    task->offset = offset;
    task->answer = answer;
    task->tid = tid;
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
    if (task == completed_task_queue_tail)
    {
        completed_task_queue_tail = NULL;
    }
    pthread_mutex_unlock(&completed_task_queue_mutex);
    return task;
}

void init_queue(const size_t queue_size)
{
    struct task *area = calloc(queue_size, sizeof(struct task));
    if (area == NULL)
        exit(EXIT_FAILURE);

    pthread_mutex_lock(&unused_area_list_mutex);
    area_head = unused_area_list_head = area;
    for (size_t i = 1; i < queue_size; i++)
    {
        area[i - 1].next = &area[i];
    }
    area[queue_size - 1].next = NULL;
    unused_area_list_tail = &area[queue_size - 1];
    pthread_mutex_unlock(&unused_area_list_mutex);
}

void free_queue()
{
    free(area_head);
    area_head = NULL;
}
