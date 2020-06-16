
#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stdio.h>

typedef struct chunk_pos
{
    int64_t seed;
    int32_t x;
    int32_t z;
} chunk_pos;
// TODO chunk_pos* を void* にする
typedef struct seedqueue
{
    chunk_pos *data;
    size_t size;
    size_t head;
    size_t tail;
} seedqueue;

int enq(seedqueue *queue, chunk_pos *pos);
chunk_pos *deq(seedqueue *queue);
seedqueue *new_queue(void);
int init_queue(seedqueue *queue, size_t size);
int destry_queue(seedqueue *queue);

#endif
