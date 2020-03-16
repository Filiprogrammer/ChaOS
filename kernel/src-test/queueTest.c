#include "queue.h"
#include "test.h"

static queue_t* queue;

void test_queue_setup() {
    queue = queue_new();
}

void test_queue_tearDown() {
    queue_destroy(queue);
}

void test_queue_enqueue_dequeue() {
    queue_enqueue(queue, (void*)1);
    queue_enqueue(queue, (void*)2);
    queue_enqueue(queue, (void*)3);

    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), 1);
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), 2);
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), 3);
    TEST_ASSERT(queue_isEmpty(queue));
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), NULL);
}

void test_queue_removeElement_first() {
    queue_enqueue(queue, (void*)1);
    queue_enqueue(queue, (void*)2);
    queue_enqueue(queue, (void*)3);

    TEST_ASSERT(queue_removeElement(queue, (void*)1));

    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), 2);
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), 3);
    TEST_ASSERT(queue_isEmpty(queue));
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), NULL);
}

void test_queue_removeElement_inbetween() {
    queue_enqueue(queue, (void*)1);
    queue_enqueue(queue, (void*)2);
    queue_enqueue(queue, (void*)3);

    TEST_ASSERT(queue_removeElement(queue, (void*)2));

    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), 1);
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), 3);
    TEST_ASSERT(queue_isEmpty(queue));
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), NULL);
}

void test_queue_removeElement_last() {
    queue_enqueue(queue, (void*)1);
    queue_enqueue(queue, (void*)2);
    queue_enqueue(queue, (void*)3);

    TEST_ASSERT(queue_removeElement(queue, (void*)3));

    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), 1);
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), 2);
    TEST_ASSERT(queue_isEmpty(queue));
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_dequeue(queue), NULL);
}

void test_queue_peek() {
    queue_enqueue(queue, (void*)1);
    queue_enqueue(queue, (void*)2);
    queue_enqueue(queue, (void*)3);

    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_peek(queue, 0), 1);
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_peek(queue, 1), 2);
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_peek(queue, 2), 3);
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_peek(queue, 3), NULL);
    TEST_ASSERT_EQUAL_UINT((uint32_t)queue_peek(queue, 4), NULL);
}

void test_queue_main() {
    test_queue_setup();
    TEST_RUN(test_queue_enqueue_dequeue);
    test_queue_tearDown();

    test_queue_setup();
    TEST_RUN(test_queue_removeElement_first);
    test_queue_tearDown();

    test_queue_setup();
    TEST_RUN(test_queue_removeElement_inbetween);
    test_queue_tearDown();

    test_queue_setup();
    TEST_RUN(test_queue_removeElement_last);
    test_queue_tearDown();

    test_queue_setup();
    TEST_RUN(test_queue_peek);
    test_queue_tearDown();
}
