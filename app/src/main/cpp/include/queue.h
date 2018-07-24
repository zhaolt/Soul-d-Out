//
// Created by ZhaoLiangtai on 2018/7/24.
//

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

#ifndef STDLIB_H
#define STDLIB_H
#include <stdlib.h>
#endif

#ifndef ITEM_H
#define ITEM_H
#include "item.h"
#endif

typedef struct QUEUE
{
    int head;
    int tail;
    int count;
    int size;
    item_t* array;
} queue_t;

int queue_init(queue_t * Q, int size);
int queue_resize(queue_t * Q, int size);
int queue_free(queue_t * Q);
int queue_empty(queue_t * Q);
int queue_enqueue(queue_t * Q, item_t item);
int queue_dequeue(queue_t * Q, item_t * item);
void queue_info(queue_t * Q);

int deque_head_insert(queue_t * Q, item_t item);
int deque_tail_insert(queue_t * Q, item_t item);
int deque_head_delete(queue_t * Q, item_t * item);
int deque_tail_delete(queue_t * Q, item_t * item);