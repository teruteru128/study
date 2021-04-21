
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "task_queue.h"

/**
 * @brief タスクキュー
 * 未着手タスクキュー
 * 
 */
static QUEUE_DEFINE(unstarted_task_queue);

/**
 * @brief 完了済みタスクキュー
 * タスク結果キュー
 *
 * completed_taskよりはresultのほうが近い気がする
 */
static QUEUE_DEFINE(completed_task_queue);

/**
 * @brief 未使用領域を複数個まとめてデキューする.
 * 残り領域数<numの場合はどのような挙動に？
 * 
 * @param num 
 * @return struct task* 
 */
struct task *unused_node_bulk_dequeue(size_t num);

/**
 * @brief 
 * 
 * @param task 
 */
void unused_node_bulk_enqueue(struct task *task);

void clear_task(struct task *task)
{
    task->offset = 0;
    task->answer = 0;
    task->tid = 0;
    task->padding = 0;
    task->index = 0;
    task->diff.tv_sec = 0;
    task->diff.tv_nsec = 0;
}

void unstarted_task_signal_not_empty()
{
    signalNotEmpty(&unstarted_task_queue);
}

size_t unstarted_task_getSize_nolock()
{
    return q_getSize_nolock(&unstarted_task_queue);
}

int unstarted_task_enqueue_putLock()
{
    pthread_mutex_lock(&unstarted_task_queue.putLock);
    pthread_mutex_lock(&unstarted_task_queue.sizeLock);
    return 0;
}

int unstarted_task_enqueue_putUnlock()
{
    pthread_mutex_unlock(&unstarted_task_queue.sizeLock);
    pthread_mutex_unlock(&unstarted_task_queue.putLock);
    return 0;
}

int unstarted_task_enqueue_nolock(struct task *task)
{
    put_nolock(&unstarted_task_queue, task);
    return 0;
}

/**
 * @brief 
 * 
 * @param task 
 * @return int 
 */
int unstarted_task_enqueue(struct task *task)
{
    if (task == NULL)
    {
        return 1;
    }
    put(&unstarted_task_queue, task);
    return 0;
}

int add_unstarted_task_nolock(unsigned int offset, size_t index)
{
    struct task *task = calloc(1, sizeof(struct task));
    // 書き込み
    task->offset = offset;
    task->answer = 0;
    task->tid = 0;
    task->padding = 0;
    task->index = index;
    task->diff.tv_sec = 0;
    task->diff.tv_nsec = 0;
    unstarted_task_enqueue_nolock(task);
    return 0;
}

/**
 * @brief 
 * 
 * @param offset 
 * @return int 
 */
int add_unstarted_task(unsigned int offset, size_t index)
{
    struct task *task = calloc(1, sizeof(struct task));
    // 書き込み
    task->offset = offset;
    task->answer = 0;
    task->tid = 0;
    task->padding = 0;
    task->index = index;
    task->diff.tv_sec = 0;
    task->diff.tv_nsec = 0;
    //タスクキューに追加
    unstarted_task_enqueue(task);
    return 0;
}

/**
 * @brief 
 * 
 * @return struct task* 
 */
struct task *unstarted_task_dequeue()
{
    return take(&unstarted_task_queue);
}

#if 0
/**
 * @brief 
 * 
 * @param offset 
 * @return int 
 */
unsigned int pop_unstarted_task()
{
    struct task *task = unstarted_task_dequeue();

    unsigned int offset = task->offset;

    free(task);
    return offset;
}
#endif

/**
 * @brief 
 * 
 * @param task 
 * @return int 
 */
int completed_task_enqueue(struct task *task)
{
    if (task == NULL)
    {
        return 1;
    }
    put(&completed_task_queue, task);
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
int add_completed_task(unsigned int offset, int answer, size_t index, const int tid, struct timespec *diff)
{
    struct task *task = calloc(1, sizeof(struct task));
    task->offset = offset;
    task->answer = answer;
    task->tid = tid;
    task->index = index;
    task->diff.tv_sec = diff->tv_sec;
    task->diff.tv_nsec = diff->tv_nsec;
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
    return take(&completed_task_queue);
}
