
#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <pthread.h>

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
    struct task *prev;
};

/* headとtailをvoid *型にしたい */
struct task_list
{
    struct task *head;
    struct task *tail;
    size_t size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void task_enqueue(struct task_list *list, struct task *task);
struct task *task_dequeue(struct task_list *list);

// まとめて取り出し/追加
void task_bulk_enqueue(struct task_list *list, struct task *task);
struct task *task_bulk_dequeue(struct task_list *list);

struct task *unused_area_dequeue();
void unused_area_enqueue(struct task *task);

int unstarted_task_enqueue(struct task *task);
int add_unstarted_task(unsigned int offset, size_t index);
struct task *unstarted_task_dequeue();
unsigned int pop_unstarted_task();

int completed_task_enqueue(struct task *task);
int add_completed_task(unsigned int offset, int answer, size_t index, const int tid, time_t timediff);
struct task *completed_task_dequeue();

// 初期化/破棄
void init_queue(const size_t queue_size);
void free_queue();

#endif
