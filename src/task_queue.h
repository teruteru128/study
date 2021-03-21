
#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

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

struct task *unused_area_dequeue();
void unused_area_enqueue(struct task *task);
int task_enqueue(struct task *task);
int add_task(unsigned int offset);
struct task *task_dequeue();
unsigned int pop_task();
int completed_task_enqueue(struct task *task);
int add_completed_task(unsigned int offset, int answer, size_t index, const int tid, time_t timediff);
struct task *completed_task_dequeue();
void init_queue(const size_t queue_size);
void free_queue();

#endif
