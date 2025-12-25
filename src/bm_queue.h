
#ifndef BM_QUEUE_H
#define BM_QUEUE

#include <pthread.h>
#include <stdbool.h>

// キューの要素
typedef struct Node
{
    void *data;
    struct Node *next;
} Node;

// スレッドセーフなキュー構造体
typedef struct
{
    Node *head;
    Node *tail;
    pthread_mutex_t mutex; // 排他制御用
    pthread_cond_t cond;   // 要素が追加されたことを通知用
    bool shutdown;         // キューを閉じるフラグ
} Queue;

extern void queue_init(Queue *q);
extern void queue_destroy(Queue *q);
extern void queue_push(Queue *q, void *data);
extern bool queue_pop(Queue *q, void **out_data);
#endif
