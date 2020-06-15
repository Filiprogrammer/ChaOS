#include "os.h"
#include "string.h"

/**
 * @brief Enable interrupts.
 * 
 */
void sti() { __asm__ volatile("sti"); }

/**
 * @brief Disable interrupts.
 * 
 */
void cli() { __asm__ volatile("cli"); }

/**
 * @brief Do nothing.
 * 
 */
void nop() { __asm__ volatile("nop"); }

/**
 * @brief Wait until next interrupt.
 * 
 */
void hlt() { __asm__ volatile("hlt"); }

/**
 * @brief Fetch the value from the 32 bit stack pointer.
 * 
 * @return uint32_t the value from the esp register.
 */
uint32_t fetchESP() {
    uint32_t esp;
    __asm__ volatile("mov %%esp, %0"
                     : "=r"(esp));
    return esp;
}

/**
 * @brief Fetch the value from the 32 bit base pointer.
 * 
 * @return uint32_t the value from the ebp register.
 */
uint32_t fetchEBP() {
    uint32_t ebp;
    __asm__ volatile("mov %%ebp, %0"
                     : "=r"(ebp));
    return ebp;
}

/**
 * @brief Fetch the value from the 16 bit stack segment.
 * 
 * @return uint32_t the value from the ss register.
 */
uint32_t fetchSS() {
    register uint32_t eax __asm__("%eax");
    __asm__ volatile("movl %ss,%eax");
    return eax;
}

/**
 * @brief Fetch the value from the 16 bit code segment.
 * 
 * @return uint32_t the value from the cs register.
 */
uint32_t fetchCS() {
    register uint32_t eax __asm__("%eax");
    __asm__ volatile("movl %cs,%eax");
    return eax;
}

/**
 * @brief Fetch the value from the 16 bit data segment.
 * 
 * @return uint32_t the value from the ds register.
 */
uint32_t fetchDS() {
    register uint32_t eax __asm__("%eax");
    __asm__ volatile("movl %ds,%eax");
    return eax;
}

/**
 * @brief Search for the most significant set bit.
 * 
 * @param value The value to be searched
 * @return uint32_t If a most significant bit is found, the index gets returned, otherwise 0.
 */
uint32_t bitScanReverse(uint32_t val) {
    uint32_t bitnr;
    __asm__("bsrl %1, %0" : "=r"(bitnr) : "r"(val));
    return bitnr;
}

/**
 * @brief Search for the least significant set bit.
 * 
 * @param value The value to be searched
 * @return uint32_t If a least significant bit is found, the index gets returned, otherwise 0.
 */
uint32_t bitScanForward(uint32_t val) {
    uint32_t bitnr;
    __asm__("bsfl %1, %0" : "=r"(bitnr) : "r"(val));
    return bitnr;
}

/**
 * @brief Read an 8 bit value from the I/O bus.
 * 
 * @param port The address on the I/O bus.
 * @return uint8_t The 8 bit value that was read.
 */
uint8_t inportb(uint16_t port) {
    uint8_t ret_val;
    __asm__ volatile ("inb %w1,%b0"	: "=a"(ret_val)	: "d"(port));
    return ret_val;
}

/**
 * @brief Read a 16 bit value from the I/O bus.
 * 
 * @param port The address on the I/O bus.
 * @return uint16_t The 16 bit value that was read.
 */
uint16_t inportw(uint16_t port) {
    uint16_t ret_val;
    __asm__ volatile ("inw %%dx,%%ax" : "=a" (ret_val) : "d" (port));
    return ret_val;
}

/**
 * @brief Read a 32 bit value from the I/O bus.
 * 
 * @param port The address on the I/O bus.
 * @return uint32_t The 32 bit value that was read.
 */
uint32_t inportl(uint16_t port) {
    uint32_t ret_val;
    __asm__ volatile ("inl %1,%0" : "=a" (ret_val) : "Nd" (port));
    return ret_val;
}

/**
 * @brief Write an 8 bit value to the I/O bus.
 * 
 * @param port The address on the I/O bus.
 * @param val The 8 bit value to be written.
 */
void outportb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %b0,%w1" : : "a"(val), "d"(port));
}

/**
 * @brief Write a 16 bit value to the I/O bus.
 * 
 * @param port The address on the I/O bus.
 * @param val The 16 bit value to be written.
 */
void outportw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %%ax,%%dx" : : "a"(val), "d"(port));
}

/**
 * @brief Write a 32 bit value to the I/O bus.
 * 
 * @param port The address on the I/O bus.
 * @param val The 32 bit value to be written.
 */
void outportl(uint16_t port, uint32_t val) {
    __asm__ volatile ("outl %0,%1" : : "a"(val), "Nd"(port));
}

void panic_assert(char* file, uint32_t line, char* desc) {
    cli();
    printf("ASSERTION FAILED(%s) at %s:%i\tOPERATING SYSTEM HALTED\n", desc, file, line);
    // Halt by going into an infinite loop.
    for (;;) {
        hlt();
    }
}

/**
 * @brief Dumps a block of memory
 * 
 * @param start pointer to the block of memory.
 * @param count number of bytes.
 */
void memshow(void* start, size_t count) {
    const uint8_t* end = (const uint8_t*)(start + count);
    for (; count != 0; count--) printf("%x ", *(end - count));
}

/**
 * @brief Copies a block of memory.
 * 
 * @param dest pointer to the destination where the content is to be copied.
 * @param src pointer to the source of data to be copied.
 * @param count number of bytes to copy.
 * @return void* dest
 */
void* memcpy(void* dest, const void* src, size_t count) {
    const uint8_t* sp = (const uint8_t*)src;
    uint8_t* dp = (uint8_t*)dest;
    for (; count != 0; count--) *dp++ = *sp++;
    return dest;
}

/**
 * @brief fills a block of memory with a byte.
 * 
 * @param dest pointer to the block of memory to fill.
 * @param val the 8 bit value to be set.
 * @param count number of bytes to be set to the value.
 * @return void* pointer to the memory area dest.
 */
void* memset(void* dest, int8_t val, size_t count) {
    char* temp = (char*)dest;
    for (; count != 0; count--) *temp++ = val;
    return dest;
}

/**
 * @brief fills a block of memory with a word.
 * 
 * @param dest pointer to the block of memory to fill.
 * @param val the 16 bit value to be set.
 * @param count nukber of words to be set to the value.
 * @return uint16_t* pointer to the memory area dest.
 */
uint16_t* memsetw(uint16_t* dest, uint16_t val, size_t count) {
    uint16_t* temp = (uint16_t*)dest;
    for (; count != 0; count--) *temp++ = val;
    return dest;
}

/**
 * @brief Computes the checksum of a string using the BSD checksum algorithm.
 * 
 * @param str string pointer to the string.
 * @return uint8_t the resulting checksum.
 */
uint8_t BSDChecksum(char* str) {
    uint8_t sum;
    uint8_t i;
    size_t len = strlen(str);
    for (sum = i = 0; i < len; ++i) {
        sum = (uint8_t)((((sum & 1) << 7) | ((sum & 0xFE) >> 1)) + (uint8_t)str[i]);
    }
    return sum;
}

static uint32_t seed = 0;

/**
 * @brief Sets the seed for the random number generator.
 * 
 * @param val seed.
 */
void randomSetSeed(uint32_t val) {
    seed = val;
}

/**
 * @brief Generates a pseudo-random number.
 * 
 * @return uint32_t a value between 0 and UINT32_MAX
 */
uint32_t random() {
    seed = (1780820173 * seed + 718214708) % 0xFFFFFFFF;
    return seed;
}

/**
 * @brief Reboots the system.
 * 
 */
void reboot() {
    int32_t temp;  // A temporary int for storing keyboard info. The keyboard is used to reboot
    do             //flush the keyboard controller
    {
        temp = inportb(0x64);
        if (temp & 1)
            inportb(0x60);
    } while (temp & 2);

    // Reboot
    outportb(0x64, 0xFE);
}

/**
 * @brief Converts uint32_t to a string.
 * 
 * @param value value to be converted.
 * @param valuestring pointer to a block of memory where to store the resulting string.
 */
void uitoa(uint32_t value, char* valuestring) {
    uint32_t min_flag;
    char swap, *p;
    min_flag = 0;

    p = valuestring;

    do {
        *p++ = (char)(value % 10) + '0';
        value /= 10;
    } while (value);

    if (min_flag != 0) {
        ++*valuestring;
    }
    *p-- = '\0';

    while (p > valuestring) {
        swap = *valuestring;
        *valuestring++ = *p;
        *p-- = swap;
    }
}

/**
 * @brief Converts int32_t to a string.
 * 
 * @param value value to be converted.
 * @param valuestring pointer to a block of memory where to store the resulting string.
 */
void itoa(int32_t value, char* valuestring) {
    int32_t min_flag;
    char swap, *p;
    min_flag = 0;

    if (0 > value) {
        *valuestring++ = '-';
        value = -INT32_MAX > value ? min_flag = INT32_MAX : -value;
    }

    p = valuestring;

    do {
        *p++ = (char)(value % 10) + '0';
        value /= 10;
    } while (value);

    if (min_flag != 0) {
        ++*valuestring;
    }
    *p-- = '\0';

    while (p > valuestring) {
        swap = *valuestring;
        *valuestring++ = *p;
        *p-- = swap;
    }
}

/**
 * @brief Converts uint32_t to a hex string.
 * 
 * @param val value to be converted.
 * @param dest pointer to a block of memory where to store the resulting string.
 * @param len number of hexadecimal digits.
 */
void i2hex(uint32_t val, char* dest, int32_t len) {
    char* cp;
    int8_t x;
    uint32_t n;
    n = val;
    cp = &dest[len];
    while (cp > dest) {
        x = n & 0xF;
        n >>= 4;
        *--cp = x + ((x > 9) ? 'A' - 10 : '0');
    }
    dest[len] = 'h';
    dest[len + 1] = '\0';
}

/**
 * @brief Converts float to a string.
 * 
 * @param value value to be converted.
 * @param decimal number of digits to be considered after the decimal point.
 * @param valuestring pointer to a block memory where to store the resulting string.
 */
void ftoa(float value, int32_t decimal, char* valuestring) {
    int32_t neg = 0;
    char tempstr[20];
    int32_t i = 0;
    int32_t j = 0;
    int32_t c;
    int32_t val1, val2;
    char* tempstring;
    tempstring = valuestring;
    if (value < 0) {
        neg = 1;
        value = -value;
    }
    for (j = 0; j < decimal; ++j) {
        value = value * 10;
    }
    val1 = (value * 2);
    val2 = (val1 / 2) + (val1 % 2);
    while (val2 != 0) {
        if ((decimal > 0) && (i == decimal)) {
            tempstr[i] = (char)(0x2E);
            ++i;
        } else {
            c = (val2 % 10);
            tempstr[i] = (char)(c + 0x30);
            val2 = val2 / 10;
            ++i;
        }
    }
    if (neg) {
        *tempstring = '-';
        ++tempstring;
    }
    i--;
    for (; i > -1; i--) {
        *tempstring = tempstr[i];
        ++tempstring;
    }
    *tempstring = '\0';
}
