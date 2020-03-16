#ifndef QUEUE_H
#define QUEUE_H

#include "stdbool.h"
#include "stdint.h"

typedef struct queue_node {
    struct queue_node* next;
    void* data;
} queue_node_t;

typedef struct {
    struct queue_node* front;
    struct queue_node* back;
} queue_t;

bool queue_destroy(queue_t* queue);
bool queue_isEmpty(queue_t* queue);
queue_t* queue_new();
void* queue_dequeue(queue_t* queue);
bool queue_enqueue(queue_t* queue, void* data);
bool queue_removeElement(queue_t* queue, void* data);
void* queue_peek(queue_t* queue, uint32_t pos);

#endif
