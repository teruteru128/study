
#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <pthread.h>
#include "queue.h"

struct task;
struct task
{
    unsigned int offset;
    int answer;
    int tid;
    int padding;
    size_t index;
    struct timespec diff;
};

// まとめて取り出し/追加
void task_bulk_enqueue(struct queue *list, struct task *task);
struct task *task_bulk_dequeue(struct queue *list);

void clear_task(struct task *);
size_t unstarted_task_getSize_nolock();
void unstarted_task_signal_not_empty();
int unstarted_task_enqueue_putLock();
int unstarted_task_enqueue_putUnlock();
int unstarted_task_enqueue(struct task *task);
int add_unstarted_task_nolock(unsigned int offset, size_t index);
int add_unstarted_task(unsigned int offset, size_t index);
struct task *unstarted_task_dequeue();
unsigned int pop_unstarted_task();

int completed_task_enqueue(struct task *task);
int add_completed_task(unsigned int offset, int answer, size_t index, const int tid, struct timespec *timediff);
struct task *completed_task_dequeue();

#endif
