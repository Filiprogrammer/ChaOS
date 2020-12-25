#include "queue.h"

#include "kheap.h"

/**
 * @brief Destroy the queue.
 * 
 * @param queue pointer to the queue
 * @return true Queue was destroyed successfully.
 * @return false Queue could not be destroyed.
 */
bool queue_destroy(queue_t* queue) {
    if (queue == NULL)
        return false;

    while (queue->front != NULL) {
        queue_node_t* node = queue->front;
        queue->front = node->next;
        free(node);
    }

    free(queue);
    return true;
}

/**
 * @brief Check whether the queue is empty.
 * 
 * @param queue pointer to the queue
 * @return true Queue is empty.
 * @return false Queue is not empty.
 */
bool queue_isEmpty(queue_t* queue) {
    if (queue == NULL || queue->front == NULL)
        return true;

    return false;
}

/**
 * @brief Create a new queue.
 * 
 * @return queue_t* pointer to the queue
 */
queue_t* queue_new() {
    queue_t* queue = malloc(sizeof(queue_t), 0);

    if (queue == NULL)
        return NULL;

    queue->front = queue->back = NULL;
    return queue;
}

/**
 * @brief Dequeue the first element in the queue.
 * 
 * @param queue pointer to the queue
 * @return void* dequeued element or NULL if queue
 */
void* queue_dequeue(queue_t* queue) {
    if (queue == NULL || queue->front == NULL)
        return NULL;

    queue_node_t* node = queue->front;
    void* data = node->data;
    queue->front = node->next;

    if (queue->front == NULL)
        queue->back = NULL;

    free(node);
    return data;
}

/**
 * @brief Add an element at the end of the queue.
 * 
 * @param queue pointer to the queue
 * @param data element to enqueue
 * @return true Element was added to queue successfully.
 * @return false Element could not be added to queue.
 */
bool queue_enqueue(queue_t* queue, void* data) {
    if (queue == NULL)
        return false;

    queue_node_t* node = malloc(sizeof(queue_node_t), 0);

    if (node == NULL)
        return false;

    node->data = data;
    node->next = NULL;

    if (queue->back == NULL) {
        queue->front = queue->back = node;
    } else {
        queue->back->next = node;
        queue->back = node;
    }

    return true;
}

/**
 * @brief Remove the first matching element from the queue.
 * 
 * @param queue pointer to the queue
 * @param data element to remove
 * @return true Element was removed successfully.
 * @return false Element could not be removed.
 */
bool queue_removeElement(queue_t* queue, void* data) {
    if (queue == NULL)
        return false;

    queue_node_t* node = queue->front;

    if (node == NULL)
        return false;

    if (node->data == data) {
        queue_node_t* temp = node;

        if (node == queue->back)
            queue->back = NULL;

        node = node->next;
        queue->front = node;
        free(temp);
        return true;
    }

    while (node->next != NULL) {
        if (node->next->data == data) {
            queue_node_t* temp = node->next;

            if (temp == queue->back)
                queue->back = node;

            node->next = node->next->next;
            free(temp);
            return true;
        }

        node = node->next;
    }

    return false;
}

/**
 * @brief Get the element at a specific position from the queue.
 * 
 * @param queue pointer to the queue
 * @param pos position of the element in the queue
 * @return void* the element or NULL if no element was found at the given position
 */
void* queue_peek(queue_t* queue, uint32_t pos) {
    if (queue == NULL || queue->front == NULL)
        return NULL;

    queue_node_t* node = queue->front;

    for (uint32_t i = 1; i <= pos; ++i) {
        if (node == NULL || node->next == NULL)
            return NULL;

        node = node->next;
    }

    return node->data;
}
