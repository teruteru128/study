
#include "bm_queue.h"
#include <stdlib.h>

// queue型定義

void queue_init(Queue *q)
{
    q->head = q->tail = NULL;
    q->shutdown = false;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void queue_destroy(Queue *q)
{
    pthread_mutex_lock(&q->mutex);
    Node *curr = q->head;
    while (curr)
    {
        Node *next = curr->next;
        free(curr); // data自体の解放は利用側で責任を持つ
        curr = next;
    }
    pthread_mutex_unlock(&q->mutex);
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
}

void queue_push(Queue *q, void *data)
{
    Node *new_node = malloc(sizeof(Node));
    new_node->data = data;
    new_node->next = NULL;

    pthread_mutex_lock(&q->mutex);
    if (q->tail == NULL)
    {
        q->head = q->tail = new_node;
    }
    else
    {
        q->tail->next = new_node;
        q->tail = new_node;
    }
    pthread_cond_signal(&q->cond); // 待機中のスレッドに通知
    pthread_mutex_unlock(&q->mutex);
}

bool queue_pop(Queue *q, void **out_data)
{
    pthread_mutex_lock(&q->mutex);

    // データが空の間は待機
    while (q->head == NULL && !q->shutdown)
    {
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    if (q->shutdown && q->head == NULL)
    {
        pthread_mutex_unlock(&q->mutex);
        return false;
    }

    Node *temp = q->head;
    *out_data = temp->data;
    q->head = q->head->next;

    if (q->head == NULL)
    {
        q->tail = NULL;
    }

    free(temp);
    pthread_mutex_unlock(&q->mutex);
    return true;
}
