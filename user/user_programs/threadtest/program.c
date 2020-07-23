#include "userlib.h"

uint32_t x = 0;

void doSomething() {
    puts("doing something\n");
    sleepMilliSeconds(500);

    for (uint32_t i = 0; i < 1000000; ++i)
        ++x;
    
    sleepMilliSeconds(700);
}

void doSomethingElse() {
    puts("doing something else\n");

    for (uint32_t i = 0; i < 1000000; ++i)
        ++x;

    sleepMilliSeconds(300);

    for (uint32_t i = 0; i < 1000000; ++i)
        ++x;
}

int main() {
    puts("Thread test\n");
    create_thread(&doSomething);
    create_thread(&doSomethingElse);
    sleepMilliSeconds(1000);
    puts("End of thread test\n");

    char x_str[11] = {0};
    uitoa(x, x_str);
    puts(x_str);
    putch('\n');

    exitCurrentThread();

    return 0;
}
