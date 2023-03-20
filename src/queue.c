
/**
 * @file queue.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2021-04-15
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "queue.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
struct node
{
    struct node *next;
    struct node *prev;
    void *item;
};

static void *dequeue(struct queue *queue)
{
    struct node *h = queue->head;

    queue->head = h->next;
    queue->head->prev = NULL;
    h->next = h;
    if (h == queue->tail)
    {
        queue->tail = NULL;
    }
    void *e = h->item;
    h->item = NULL;
    free(h);
    return e;
}

static void enqueue(struct queue *queue, struct node *node)
{
    if (queue->head == NULL)
    {
        queue->head = node;
    }
    if (queue->tail != NULL)
    {
        queue->tail->next = node;
    }
    queue->tail = node;
}

void signalNotEmpty(struct queue *queue)
{
    pthread_mutex_lock(&queue->takeLock);
    pthread_cond_broadcast(&queue->notEmpty);
    pthread_mutex_unlock(&queue->takeLock);
}

void signalNotFull(struct queue *queue)
{
    pthread_mutex_lock(&queue->putLock);
    pthread_cond_signal(&queue->notFull);
    pthread_mutex_unlock(&queue->putLock);
}

void put_nolock(struct queue *queue, void *e)
{
    struct node *node = calloc(1, sizeof(struct node));
    node->item = e;
    enqueue(queue, node);
    queue->size++;
}

void put(struct queue *queue, void *e)
{
    if (queue == NULL || e == NULL)
    {
        return;
    }

    struct node *node = calloc(1, sizeof(struct node));
    node->item = e;

    pthread_mutex_lock(&queue->putLock);

    enqueue(queue, node);

    pthread_rwlock_wrlock(&queue->sizeLock);
    size_t c = queue->size++;
    pthread_rwlock_unlock(&queue->sizeLock);

    if ((c + 1) < queue->capacity)
        pthread_cond_signal(&queue->notFull);
    pthread_mutex_unlock(&queue->putLock);
    if (c == 0)
        signalNotEmpty(queue);
}

void *take(struct queue *queue)
{
    if (queue == NULL)
    {
        return NULL;
    }
    size_t c = (size_t)-1;

    pthread_mutex_lock(&queue->takeLock);

    pthread_rwlock_rdlock(&queue->sizeLock);
    while (queue->size == 0)
    {
        pthread_rwlock_unlock(&queue->sizeLock);
        pthread_cond_wait(&queue->notEmpty, &queue->takeLock);
        pthread_rwlock_rdlock(&queue->sizeLock);
    }
    pthread_rwlock_rdlock(&queue->sizeLock);

    void *item = dequeue(queue);
    pthread_rwlock_wrlock(&queue->sizeLock);
    queue->size--;
    c = queue->size;
    pthread_rwlock_unlock(&queue->sizeLock);
    if (c >= 1)
    {
        pthread_cond_signal(&queue->notEmpty);
    }
    pthread_mutex_unlock(&queue->takeLock);
    if (c == queue->capacity)
        signalNotFull(queue);
    return item;
}

size_t q_getSize(struct queue *queue)
{
    pthread_rwlock_rdlock(&queue->sizeLock);
    size_t size = queue->size;
    pthread_rwlock_unlock(&queue->sizeLock);
    return size;
}

int q_isEmpty(struct queue *queue)
{
    pthread_rwlock_rdlock(&queue->sizeLock);
    int isEmpty = !queue->size;
    pthread_rwlock_unlock(&queue->sizeLock);
    return isEmpty;
}

size_t q_getSize_nolock(struct queue *queue) { return queue->size; }

int q_isEmpty_nolock(struct queue *queue) { return !queue->size; }

/**
 * TODO: キューに制限値を導入する
 * @brief
 * キューを生成して返します。
 *
 * @param capacity
 * @return struct queue*
 */
struct queue *queue_new(const size_t capacity)
{
    struct queue *queue = malloc(sizeof(struct queue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->capacity = capacity;
    queue->size = 0;
    pthread_rwlock_init(&queue->sizeLock, NULL);
    pthread_mutex_init(&queue->takeLock, NULL);
    pthread_cond_init(&queue->notEmpty, NULL);
    pthread_mutex_init(&queue->putLock, NULL);
    pthread_cond_init(&queue->notFull, NULL);
    queue->destructor = NULL;
    return queue;
}

static void nodes_destory(struct queue *queue)
{
    struct node *ptr = queue->head;
    while (ptr != NULL)
    {
        struct node *current = ptr;
        if (queue->destructor)
            queue->destructor(current->item);
        current->item = NULL;
        ptr = current->next;
        ptr->prev = NULL;
        current->next = NULL;
        free(current);
    }
    queue->head = NULL;
    queue->tail = NULL;
}

void set_destructor(struct queue *queue, void (*dest)(void *))
{
    queue->destructor = dest;
}

void queue_free(struct queue *queue)
{
    nodes_destory(queue);
    queue->capacity = 0;
    queue->size = 0;
    pthread_rwlock_destroy(&queue->sizeLock);
    pthread_mutex_destroy(&queue->takeLock);
    pthread_cond_destroy(&queue->notEmpty);
    pthread_mutex_destroy(&queue->putLock);
    pthread_cond_destroy(&queue->notFull);
}
