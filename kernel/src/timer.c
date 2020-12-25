#include "math.h"
#include "os.h"

uint16_t systemfrequency;
uint32_t timer_ticks = 0;

uint64_t lastRdtscValue = 0;
uint32_t currentSeconds = 0xFFFFFFFF;

void timer_handler(registers_t* r) {
    ++timer_ticks;

    // TODO: Put this block of code somewhere else
    if (timer_getSeconds() != currentSeconds) {
        currentSeconds = timer_getSeconds();

        uint64_t Rdtsc = rdtsc();
        uint64_t RdtscDiff = Rdtsc - lastRdtscValue;
        lastRdtscValue = Rdtsc;
        ODA.cpu_frequency = RdtscDiff;
    }
}

/**
 * @brief Gets the total time passed in seconds.
 * 
 * @return uint32_t seconds passed
 */
uint32_t timer_getSeconds() {
    return timer_ticks / systemfrequency;
}

/**
 * @brief Gets the total time passed in milliseconds.
 * 
 * @return uint32_t milliseconds passed
 */
uint32_t timer_getMilliseconds() {
    return (timer_ticks * 1000) / systemfrequency;
}

/**
 * @brief Enters a hlt loop for the given amount of ticks.
 * 
 * @param ticks number of ticks to wait
 */
static void timer_wait(uint32_t ticks) {
    sti();
    uint32_t eticks = 0;
    eticks = ticks + timer_ticks;
    while (eticks > timer_ticks)
        hlt();
}

/**
 * @brief Enters a hlt loop for the given amount of seconds.
 * 
 * @param seconds number of seconds to wait
 */
void sleepSeconds(uint32_t seconds) {
    timer_wait((uint32_t)100 * seconds);
}

/**
 * @brief Enters a hlt loop for the given amount of milliseconds.
 * 
 * @param ms number of milliseconds to wait
 */
void sleepMilliSeconds(uint32_t ms) {
    timer_wait((uint32_t)(ms / 10));
}

/**
 * @brief Enters a nop loop for the given amount of microseconds.
 * 
 * @param microsec number of microseconds to wait
 */
void sleepMicroSeconds(uint32_t microsec) {
    uint64_t timeout = microsec * ODA.cpu_frequency;
    div64_32(&timeout, 1000000);
    timeout += rdtsc();
    while (rdtsc() < timeout)
        nop();
}

uint32_t cpuCyclesToMicroSeconds(uint64_t cycles) {
    uint64_t cpuCyclesPerMicroSec = ODA.cpu_frequency;
    div64_32(&cpuCyclesPerMicroSec, 1000000);
    div64_32(&cycles, (uint32_t)cpuCyclesPerMicroSec);
    return cycles;
}

void systemTimer_setFrequency(uint32_t freq) {
    uint32_t divisor = 1193180 / freq;  //divisor must fit into 16 bits

    systemfrequency = freq;

    // Send the command byte
    outportb(0x43, 0x36);

    // Send divisor
    outportb(0x40, (uint8_t)(divisor & 0xFF));         // low  byte
    outportb(0x40, (uint8_t)((divisor >> 8) & 0xFF));  // high byte
}

void timer_install() {
    irq_install_handler(0, timer_handler);
    systemTimer_setFrequency(100);  // 100 Hz, meaning a tick every 10 milliseconds
}

void timer_uninstall() {
    irq_uninstall_handler(0, timer_handler);
}

/**
 * @brief Reads the time-stamp counter of the processor. (number of clock cycles since the last reset)
 * 
 * @return uint64_t number of clock cycles
 */
uint64_t rdtsc() {
    uint64_t val;
    __asm__ volatile("rdtsc"
                     : "=A"(val));  // "=A" for getting 64 bit value
    return val;
}
