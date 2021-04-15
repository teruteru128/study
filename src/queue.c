
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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
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
    h->next = NULL;
    if (h == queue->tail)
    {
        queue->tail = NULL;
    }
    h->next = NULL;
    void *e = h->item;
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

static void signalNotEmpty(struct queue *queue)
{
    pthread_mutex_lock(&queue->takeLock);
    pthread_cond_broadcast(&queue->notEmpty);
    pthread_mutex_unlock(&queue->takeLock);
}

static void signalNotFull(struct queue *queue)
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
    size_t c = queue->size++;
    if (c == 0)
        signalNotEmpty(queue);
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
    pthread_mutex_lock(&queue->sizeLock);
    size_t c = queue->size++;
    pthread_mutex_unlock(&queue->sizeLock);
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
    pthread_mutex_lock(&queue->sizeLock);
    while (queue->size == 0)
    {
        pthread_mutex_unlock(&queue->sizeLock);
        pthread_cond_wait(&queue->notEmpty, &queue->takeLock);
        pthread_mutex_lock(&queue->sizeLock);
    }
    pthread_mutex_unlock(&queue->sizeLock);

    void *item = dequeue(queue);
    pthread_mutex_lock(&queue->sizeLock);
    c = queue->size--;
    pthread_mutex_unlock(&queue->sizeLock);
    if (c > 1)
    {
        pthread_cond_signal(&queue->notEmpty);
    }
    pthread_mutex_unlock(&queue->takeLock);
    if (c == queue->capacity)
        signalNotFull(queue);
    return item;
}
