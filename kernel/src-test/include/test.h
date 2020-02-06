#ifndef TEST_H
#define TEST_H

#include "stdint.h"
#include "stdbool.h"

void test_fail(const char* file, uint32_t line, const char* function);

extern bool test_failed;

#define TEST_ASSERT_EQUAL_UINT(expected, actual) {\
                            uint32_t _expected = expected;\
                            uint32_t _actual = actual;\
                            if (_expected != _actual) {\
                                test_fail(__FILE__, __LINE__, __FUNCTION__);\
                                printf("Expected %u was %u\n", _expected, _actual);\
                                return;\
                            }\
                        }

#define TEST_ASSERT_EQUAL_INT(expected, actual) {\
                            int32_t _expected = expected;\
                            int32_t _actual = actual;\
                            if (_expected != _actual) {\
                                test_fail(__FILE__, __LINE__, __FUNCTION__);\
                                printf("Expected %d was %d\n", _expected, _actual);\
                                return;\
                            }\
                        }

#define TEST_ASSERT(condition) {\
                            if (!(condition)) {\
                                test_fail(__FILE__, __LINE__, __FUNCTION__);\
                                printf("Assertion failed(%s)\n", #condition);\
                                return;\
                            }\
                        }

#define TEST_RUN(func, ...) {\
                        test_failed = false;\
                        func(__VA_ARGS__);\
                        if (test_failed)\
                            printf("%s failed\n", #func);\
                        else\
                            printf("%s passed\n", #func);\
                    }\

#endif
