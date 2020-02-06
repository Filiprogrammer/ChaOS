#include "test.h"
#include "os.h"

bool test_failed = false;

void test_fail(const char* file, uint32_t line, const char* function) {
    printf("%s:%u:%s:FAIL:", file, line, function);
    test_failed = true;
}
