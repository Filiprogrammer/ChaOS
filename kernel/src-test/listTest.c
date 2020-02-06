#include "test.h"
#include "list.h"

static listHead_t* list;

void test_list_setup() {
    list = list_create();
}

void test_list_tearDown() {
    list_deleteAllWithoutData(list);
}

void test_list_getElement() {
    list_append(list, (void*)0x12345678);
    list_append(list, (void*)0x89ABCDEF);
    list_append(list, (void*)-120);

    TEST_ASSERT_EQUAL_UINT((uint32_t)list_getElement(list, 1), 0x12345678);
    TEST_ASSERT_EQUAL_UINT((uint32_t)list_getElement(list, 2), 0x89ABCDEF);
    TEST_ASSERT_EQUAL_INT((int32_t)list_getElement(list, 3), -120);
}

void test_list_getSize() {
    list_append(list, (void*)78);
    list_append(list, (void*)0x2067D76F);
    list_append(list, (void*)-120);

    TEST_ASSERT_EQUAL_UINT(list_getSize(list), 3);
}

void test_list_main() {
    test_list_setup();
    TEST_RUN(test_list_getElement);
    test_list_tearDown();

    test_list_setup();
    TEST_RUN(test_list_getSize);
    test_list_tearDown();
}
