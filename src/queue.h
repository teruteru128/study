
#ifndef QUEUE_H
#define QUEUE_H

#include <limits.h>
#include <pthread.h>

struct node;

/* headとtailをvoid *型にしてstruct task型とstruct node型を分離したい */
struct queue
{
    struct node *head;
    struct node *tail;
    size_t capacity;
    size_t size;
    pthread_mutex_t sizeLock;
    pthread_mutex_t takeLock;
    pthread_cond_t notEmpty;
    pthread_mutex_t putLock;
    pthread_cond_t notFull;
};

#define QUEUE_INITIALIZER2(MAX) {NULL, NULL, MAX, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER}
#define QUEUE_INITIALIZER QUEUE_INITIALIZER2(INT_MAX)
#define QUEUE_DECLARE(name) struct queue name
#define QUEUE_DECLARE2(name, MAX) struct queue name
#define QUEUE_DEFINE(name) QUEUE_DECLARE(name) = QUEUE_INITIALIZER
#define QUEUE_DEFINE2(name, MAX) QUEUE_DECLARE2(name, MAX) = QUEUE_INITIALIZER2(MAX)

void put_nolock(struct queue *list, void *node);
void put(struct queue *list, void *node);
void *take(struct queue *list);
size_t q_getSize_nolock(struct queue *queue);
void signalNotEmpty(struct queue *queue);
void signalNotFull(struct queue *queue);

#endif
