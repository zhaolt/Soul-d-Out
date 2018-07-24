//
// Created by ZhaoLiangtai on 2018/7/24.
//
#include "queue.h"

int queue_init(queue_t *Q, int size) {
    Q->head = 0;
    Q->tail = 0;
    Q->count = 0;
    Q->array = (item_t *) calloc(size, sizeof(item_t));
    if (Q->array != NULL) {
        Q->size = size;
        return 1;
    } else {
        Q->size = 0;
        fprintf(stderr, "Queue init fail.\n");
        return 0;

    }
}

int queue_resize(queue_t *Q, int size) {
    queue_free(Q);
    return queue_init(Q, size);
}

int queue_free(queue_t *Q) {
    free(Q->array);
    Q->head = 0;
    Q->tail = 0;
    Q->count = 0;
    Q->size = 0;
    Q->array = NULL;
    return 1;
}

int queue_empty(queue_t *Q) {
    if (Q->array == NULL) {
        fprintf(stderr, "Queue is not initialized.\n");
        return 1;
    }
    if (Q->count == 0)
        return 1;
    else return 0;
}

int queue_enqueue(queue_t *Q, item_t item) {
    if (Q->array == NULL) {
        fprintf(stderr, "Queue is not initialized.\n");
        return 0;
    }
    if (Q->count < Q->size) {
        Q->array[Q->head] = item;
        Q->head = (Q->head + 1) % Q->size;
        Q->count++;
        return 1;
    } else {
        fprintf(stderr, "Queue overflow.\n");
        return 0;
    }
}

int queue_dequeue(queue_t *Q, item_t *item) {
    if (Q->array == NULL) {
        fprintf(stderr, "Queue is not initialized.\n");
        return 0;
    }
    if (!queue_empty(Q)) {
        *item = Q->array[Q->tail];
        Q->tail = (Q->tail + 1) % Q->size;
        Q->count--;
        return 1;
    } else {
        fprintf(stderr, "Queue underflow.\n");
        return 0;
    }
}

void queue_info(queue_t *Q) {
    static int count = 0;
    printf("%d:\n", count++);
    if (Q->array == NULL) {
        fprintf(stderr, "queue is not initialized.\n-------------------------------\n");
        return;
    }
    printf("queue head location:%d\n", Q->head);
    printf("queue tail location:%d\n", Q->tail);
    printf("queue count:%d\n", Q->count);
    printf("queue size:%d\n", Q->size);
    for (int i = 0; i < Q->size; i++) {
        printf("%d:%d ", i, Q->array[i].key);
    }
    printf("\n-------------------------------\n");
}

int deque_head_insert(queue_t * Q, item_t item) {
    return queue_enqueue(Q, item);
}
int deque_tail_insert(queue_t * Q, item_t item) {
    if (Q->array == NULL) {
        fprintf(stderr, "Queue is not initialized.\n");
        return 0;
    }
    if (Q->count < Q->size) {
        if (Q->count != 0) {
            Q->tail = (Q->tail - 1) % Q->size;
            if (Q->tail < 0) {
                Q->tail += Q->size;
            }
            Q->array[Q->tail] = item;
            Q->count++;
            return 1;
        } else {
            Q->count = 1;
            Q->tail = 0;
            Q->head = 1;
            Q->array[Q->tail] = item;
            return 1;
        }
    } else {
        fprintf(stderr, "Queue overflow.\n");
        return 0;
    }
}

int deque_head_delete(queue_t * Q, item_t * item) {
    if (Q->array == NULL) {
        fprintf(stderr, "Queue is not initialized.\n");
        return 0;
    }
    if (!queue_empty(Q)) {
        Q->head = (Q->head - 1) % Q->size;
        if (Q->head < 0) {
            Q->head += Q->size;
        }
        *item = Q->array[Q->head];
        Q->count--;
        return 1;
    } else {
        fprintf(stderr, "Queue underflow.\n");
        return 0;
    }
}
int deque_tail_delete(queue_t * Q, item_t * item) {
    return queue_dequeue(Q, item);
}