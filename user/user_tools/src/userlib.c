#include "userlib.h"

static uint32_t randomSeed = 0;

/// syscalls ///

void settextcolor(uint8_t foreground, uint8_t background) {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(2), "b"(foreground), "c"(background));
}

void putch(char val) {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(1), "b"(val));
}

void puts(const char* str) {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(0), "b"(str));
}

void sleepMilliSeconds(uint32_t ms) {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(4), "b"(ms));
}

bool isKeyDown(KEY_t key) {
    bool ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(6), "b"(key));
    return ret;
}

KEY_t getkey() {
    KEY_t ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(7));
    return ret;
}

bool video_set_mode(uint8_t mode) {
    bool ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(8), "b"(mode));
    return ret;
}

void put_pixel(int16_t x, int16_t y) {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(9), "b"(x), "c"(y));
}

uint8_t file_createDirectory(char* filepath) {
    uint8_t ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(10), "b"(filepath));
    return ret;
}

uint8_t file_create(file_t* file_inst, char* filepath) {
    uint8_t ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(11), "b"(file_inst), "c"(filepath));
    return ret;
}

void move_cursor_left() {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(12));
}

void move_cursor_right() {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(13));
}

uint8_t file_findByIndex(file_t* file_inst, char* dirpath, uint32_t index) {
    uint8_t ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(14), "b"(file_inst), "c"(dirpath), "d"(index));
    return ret;
}

uint8_t file_isDirectory(char* filepath) {
    uint8_t ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(15), "b"(filepath));
    return ret;
}

void desktop_enable() {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(16));
}

uint8_t file_find(file_t* file_inst, char* filepath) {
    uint8_t ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(17), "b"(file_inst), "c"(filepath));
    return ret;
}

void file_readContents(file_t* file_inst, uint8_t* buffer, uint32_t start, uint32_t len) {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(18), "b"(file_inst), "c"(buffer), "d"(start), "S"(len));
}

void reboot() {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(19));
}

void draw_picture(uint8_t* picture, int16_t x, int16_t y) {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(20), "b"(picture), "c"(x), "d"(y));
}

uint8_t file_delete(char* filepath) {
    uint8_t ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(21), "b"(filepath));
    return ret;
}

char keyToASCII(KEY_t key) {
    char ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(22), "b"(key));
    return ret;
}

uint32_t getMilliseconds() {
    uint32_t ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(23));
    return ret;
}

void randomize() {
    __asm__ volatile("int $0x7F"
                     : "=a"(randomSeed)
                     : "a"(24));
}

void getBootPath(char* filepath) {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(25), "b"(filepath));
}

void exit() {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(26));

    for (;;)
        __asm__ volatile("nop");
}

uint8_t file_execute(char* filepath) {
    uint8_t ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(27), "b"(filepath));
    return ret;
}

bool pci_getDevice(uint32_t i, pciDev_t* pciDev) {
    bool ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(28), "b"(i), "c"(pciDev));
    return ret;
}

bool create_thread(void* entry) {
    bool ret;
    __asm__ volatile("int $0x7F"
                     : "=a"(ret)
                     : "a"(29), "b"(entry));
    return ret;
}

void exitCurrentThread() {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(30));

    for (;;)
        __asm__ volatile("nop");
}

/// user functions ///

/**
 * @brief Compare two strings.
 * 
 * @param s1 first string to be compared
 * @param s2 second string to be compared
 * @return int32_t less than 0 if s1 is less than s2, 0 if they are equal or 1 otherwise.
 */
int32_t strcmp(const char* s1, const char* s2) {
    while ((*s1) && (*s1 == *s2)) {
        ++s1;
        ++s2;
    }
    return (*s1 - *s2);
}

/**
 * @brief Check if the beginning of a string matches a specified string.
 * 
 * @param str string to get checked
 * @param prefix string to look for
 * @return true beginning of str matches prefix.
 * @return false beginning of str does not match prefix.
 */
bool strstarts(const char* str, const char* prefix) {
    while (*prefix)
        if (*prefix++ != *str++)
            return false;

    return true;
}

void substring(char* str, char* sub, int pos, int len) {
    int c = 0;
    while (c < len) {
        sub[c] = str[pos + c];
        ++c;
    }
    sub[c] = '\0';
}

/**
 * @brief Compute the length of a string (not including the null terminator).
 * 
 * @param str pointer to the string
 * @return size_t length of the string
 */
size_t strlen(const char* str) {
    size_t retval;
    for (retval = 0; *str != '\0'; ++str)
        ++retval;
    return retval;
}

/**
 * @brief Copy the string pointed to by src to dest.
 * 
 * @param dest pointer to destination where the content is to be copied to
 * @param src the string to be copied
 * @return char* pointer to the destination string
 */
char* strcpy(char* dest, const char* src) {
    do {
        *dest++ = *src++;
    } while (*src);
    return dest;
}

/**
 * @brief Append the string pointed to by src to the end of the string pointed to by dest.
 * 
 * @param dest pointer to the destination array, which should be large enough to contain the concatenated resulting string including the null terminator.
 * @param src string to be appended
 * @return char* a pointer to the resulting string dest
 */
char* strcat(char* dest, const char* src) {
    char* tmp = dest;
    while (*dest)
        dest++;

    do {
        *dest++ = *src++;
    } while (*src);
    return tmp;
}

/**
 * @brief Break string str into a series of tokens using the delimiter delim.
 * 
 * @param str string to truncate (Note that the string is being modified.)
 * @param delim string containing the delimiters
 * @return char* pointer to the first token found in the string
 */
char* strtok(char* str, const char* delim) {
    static char* _buffer;
    static char _reachedEnd;
    if (str != 0) {
        _buffer = str;
        _reachedEnd = 0;
    }
    if (_buffer[0] == '\0') return 0;
    if (_reachedEnd) return 0;

    char *ret = _buffer, *b;
    const char* d;

    for (b = _buffer; *b != '\0'; ++b) {
        for (d = delim; *d != '\0'; ++d) {
            if (*b == *d) {
                *b = '\0';
                _buffer = b + 1;

                // skip the beginning delimiters
                if (b == ret) {
                    ret++;
                    continue;
                }
                return ret;
            }
        }
    }
    _reachedEnd = 1;
    return ret;
}

/**
 * @brief Remove leading and trailing white-space characters.
 * 
 * @param str string to be trimmed
 * @return char* pointer to the first non white-space charater of the string
 */
char* strtrim(char* str) {
    char* end;

    // Trim leading space
    while (((unsigned char)*str) == 0x20) str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && ((unsigned char)*end) == 0x20) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

/**
 * @brief Remove leading white-space characters.
 * 
 * @param str string to be trimmed
 * @return char* pointer to the first non white-space charater of the string
 */
char* strtrimstart(char* str) {
    // Trim leading space
    while (((unsigned char)*str) == 0x20) str++;
    return str;
}

/**
 * @brief Remove trailing white-space characters.
 * 
 * @param str string to be trimmed
 * @return char* pointer to the string str
 */
char* strtrimend(char* str) {
    char* end;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end >= str && ((unsigned char)*end) == 0x20) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

/**
 * @brief Convert all letters of a string to lower case.
 * 
 * @param str string to be converted
 * @return char* pointer to the converted string str
 */
char* strlwr(char* str) {
    char* tmp = str;

    while (*str != '\0') {
        if (*str >= 65 && *str <= 90)
            *str = *str + 32;

        ++str;
    }

    return tmp;
}

/**
 * @brief Find the first occurrence of substring in string.
 * 
 * @param string the string to be scanned
 * @param substring the string containing the sequence of characters to match
 * @return char* pointer to the first occurrence of substring in string
 */
char* strstr(const char* string, const char* substring) {
    const char *a, *b;

    b = substring;
    if (*b == 0)
        return (char*)string;

    for (; *string != 0; ++string) {
        if (*string != *b)
            continue;

        a = string;
        while (1) {
            if (*b == 0)
                return (char*)string;

            if (*a++ != *b++)
                break;
        }
        b = substring;
    }
    return NULL;
}

/**
 * @brief Reverse the given string.
 * 
 * @param str the string to be reversed
 * @return char* pointer to the reversed string
 */
char* strrev(char* str) {
    if (!str || !*str)
        return str;

    char* p1 = str;
    char* p2 = str + strlen(str) - 1;
    for (; p2 > p1; ++p1, --p2) {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }

    return str;
}

char* file_squashPath(char* filepath) {
    char delimiter[] = "/\\";
#define FILE_MAX_PATH_FILES 16
#define FILE_MAX_FNAME_LEN 34
    char spl_path[FILE_MAX_PATH_FILES][FILE_MAX_FNAME_LEN];
    uint16_t ret_arr_size = strlen(filepath) + 1;
    uint32_t i, j;
    for (i = 0; i < FILE_MAX_PATH_FILES; ++i) {
        for (j = 0; j < FILE_MAX_FNAME_LEN; ++j) {
            spl_path[i][j] = 0;
        }
    }
    uint32_t path_size = 0;
    char* cur_file = strtok(filepath, delimiter);
    while (cur_file != 0 && path_size < FILE_MAX_PATH_FILES) {
        if (strlen(cur_file) > FILE_MAX_FNAME_LEN - 1) break;  //make sure current filename is not to long
        strcpy(spl_path[path_size], cur_file);
        cur_file = strtok(0, delimiter);
        ++path_size;
    }
    for (i = 0; i < path_size; ++i) {
        if (strcmp(spl_path[i], ".") == 0) {  //if spl_path[i] equals "."
            strcpy(spl_path[i], "");
            continue;
        }
        if (strcmp(spl_path[i], "..") == 0) {  //if spl_path[i] equals ".."
            if (i > 0) {
                for (j = i - 1; j >= 0; --j) {
                    if (strlen(spl_path[j]) != 0) {
                        strcpy(spl_path[j], "");
                        break;
                    }
                }
            }
            strcpy(spl_path[i], "");
            continue;
        }
    }

    for (i = 0; i < ret_arr_size; ++i) {
        filepath[i] = 0;
    }
    for (i = 0; i < path_size; ++i) {
        if (strcmp(spl_path[i], "")) {        //if spl_path[i] does NOT equal to ""
            if (strcmp(filepath, "") == 0) {  //if filepath.Length is 0
                strcpy(filepath, spl_path[i]);
            } else {
                strcat(filepath, "/");
                strcat(filepath, spl_path[i]);
            }
        }
    }
    return filepath;
}

uint8_t __ctzdi2(uint64_t val) {
    uint32_t val_high = val >> 32;
    uint32_t bitnr;
    __asm__("bsf %1, %0"
            : "=r"(bitnr)
            : "r"((uint32_t)val));

    if (val_high != 0 && ((uint32_t)val) == 0) {
        __asm__("bsf %1, %0"
                : "=r"(bitnr)
                : "r"(val_high));
        bitnr += 32;
    }

    return bitnr;
}

int32_t __clrsbsi2(int32_t x) {
    if (x & 0x80000000)
        x = ~x;

    if (x == 0)
        return 31;

    return __builtin_clz(x) - 1;
}

static inline uint64_t __attribute__((always_inline)) __udivdi3_inline(uint64_t dividend, uint64_t divisor) {
    const int power_of_two_factor = __builtin_ctzll(divisor);
    divisor >>= power_of_two_factor;
    dividend >>= power_of_two_factor;

    // Check for division by power of two or division by zero.
    if (divisor <= 1) {
        if (divisor == 0)
            // Division by zero
            return 0;

        return dividend;
    }

    const int divisor_clz = __builtin_clzll(divisor);
    const uint64_t max_divisor = divisor << divisor_clz;
    const uint64_t max_qbit = 1ULL << divisor_clz;

    uint64_t quotient = 0;
    uint64_t remainder = dividend;

    while (remainder >= divisor) {
        int shift = __builtin_clzll(remainder);
        uint64_t scaled_divisor = max_divisor >> shift;
        uint64_t quotient_bit = max_qbit >> shift;

        int too_big = (scaled_divisor > remainder);
        scaled_divisor >>= too_big;
        quotient_bit >>= too_big;
        remainder -= scaled_divisor;
        quotient |= quotient_bit;
    }

    return quotient;
}

uint64_t __udivdi3(uint64_t dividend, uint64_t divisor) {
    return __udivdi3_inline(dividend, divisor);
}

int64_t __divdi3(int64_t dividend, int64_t divisor) {
    uint64_t n = __udivdi3_inline(llabs(dividend), llabs(divisor));
    if ((dividend ^ divisor) < 0)
        n = -n;

    return (int64_t)n;
}

static inline uint64_t __attribute__((always_inline)) __umoddi3_inline(uint64_t dividend, uint64_t divisor) {
    // Shortcircuit mod by a power of two (and catch mod by zero).
    const uint64_t mask = divisor - 1;
    if ((divisor & mask) == 0) {
        if (divisor == 0)
            // Mod by zero
            return 0;

        return dividend & mask;
    }

    const uint64_t max_divisor = divisor << __builtin_clzll(divisor);

    uint64_t remainder = dividend;
    while (remainder >= divisor) {
        const int shift = __builtin_clzll(remainder);
        uint64_t scaled_divisor = max_divisor >> shift;
        scaled_divisor >>= (scaled_divisor > remainder);
        remainder -= scaled_divisor;
    }

    return remainder;
}

uint64_t __umoddi3(uint64_t dividend, uint64_t divisor) {
    return __umoddi3_inline(dividend, divisor);
}

int64_t __moddi3(int64_t dividend, int64_t divisor) {
    uint64_t remainder = __umoddi3_inline(llabs(dividend), llabs(divisor));
    return (int64_t)((dividend >= 0) ? remainder : -remainder);
}

/**
 * @brief Return the absolute value of i.
 * 
 * @param i integral value
 * @return int32_t absolute value
 */
int32_t abs(int32_t i) {
    return i < 0 ? -i : i;
}

/**
 * @brief Return the absolute value of i.
 * 
 * @param i integral value
 * @return int64_t absolute value
 */
int64_t llabs(int64_t i) {
    return i < 0 ? -i : i;
}

int32_t ipow(int32_t base, int32_t exp) {
    int32_t result = 1;

    if (exp < 0) {
        if (base == 1)
            return 1;

        if (base == -1)
            return -1;

        return 0;
    }

    switch (31 - __builtin_clrsb(exp)) {
        case 255:
            if (base == 1)
                return 1;

            if (base == -1)
                return 1 - 2 * (exp & 1);

            // Return 0 on overflow/underflow
            return 0;
        case 5:
            if (exp & 1)
                result *= base;

            exp >>= 1;
            base *= base;
        case 4:
            if (exp & 1)
                result *= base;

            exp >>= 1;
            base *= base;
        case 3:
            if (exp & 1)
                result *= base;

            exp >>= 1;
            base *= base;
        case 2:
            if (exp & 1)
                result *= base;

            exp >>= 1;
            base *= base;
        case 1:
            if (exp & 1)
                result *= base;
        default:
            return result;
    }
}

double pow(double base, double exp) {
    double result = 0.0;

    __asm__ volatile(
        "fyl2x \n"
        "fld %%st(0) \n"
        "frndint \n"
        "fsubr %%st, %%st(1) \n"
        "fxch \n"
        "f2xm1 \n"
        "fld1 \n"
        "faddp %%st, %%st(1) \n"
        "fscale \n"
        : "=t"(result)
        : "0"(base), "u"(exp));

    return result;
}

double fabs(double x) {
    *(int*)&x &= 0x7fffffff;
    return x;
}

int32_t fact(int32_t n) {
    return n <= 0 ? 1 : n * fact(n - 1);
}

float floor(float x) {
    if (x > 0) return (int)x;
    return (int)(x - 0.9999999999999999);
}

float fmod(float a, float b) {
    return ((((a / b) - (floor(a / b))) * b) + 0.5);
}

float sin(float deg) {
    float rad = deg * PI / 180;
    float result;
    __asm__("fsin"
            : "=t"(result)
            : "0"(rad));
    return result;
}

float cos(float deg) {
    return sin(deg + 90);
}

/**
 * @brief Convert uint32_t to a string.
 * 
 * @param value value to be converted
 * @param str pointer to a block of memory where to store the resulting string
 * @param base numerical base used to represent the value as a string (between 2 and 36)
 */
void uitoa(uint32_t value, char* str, int32_t base) {
    if (base < 2 || base > 36) {
        str[0] = 0;
        return;
    }

    char* p = str;

    do {
        char digit = '0' + (value % base);

        if (digit > '9')
            digit += 'a' - '9' - 1;

        *p++ = digit;
        value /= base;
    } while (value);

    *p-- = '\0';
    strrev(str);
}

/**
 * @brief Convert int32_t to a string.
 * 
 * @param value value to be converted
 * @param str pointer to a block of memory where to store the resulting string
 * @param base numerical base used to represent the value as a string (between 2 and 36)
 */
void itoa(int32_t value, char* str, int32_t base) {
    if (base < 2 || base > 36) {
        str[0] = 0;
        return;
    }

    bool neg = false;

    if (value < 0) {
        *str++ = '-';
        value = -value;
        neg = true;
    }

    char* p = str;

    do {
        char digit = '0' + (value % base);

        if (digit > '9')
            digit += 'a' - '9' - 1;

        *p++ = digit;
        value /= base;
    } while (value);

    if (neg)
        ++*str;

    *p-- = '\0';
    strrev(str);
}

void ftoa(float f, char* buffer) {
    if (f < 0)
        *(buffer++) = '-';
    int32_t i = (int32_t)f;
    itoa(i < 0 ? -i : i, buffer, 10);

    if (f < 0.0f)
        f = -f;

    buffer += strlen(buffer);
    *buffer = '.';
    ++buffer;

    *buffer++ = ((uint32_t)(f * 10.0f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 100.0f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 1000.0f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 10000.0f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 100000.0f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 1000000.0f) % 10) + '0';
    *buffer = '\0';
}

double sqrt(double x) {
    if (x < 0.0)
        return NAN;

    double result;
    __asm__("fsqrt"
            : "=t"(result)
            : "0"(x));
    return result;
}

uint32_t random() {
    randomSeed = (1780820173 * randomSeed + 718214708) % 0xFFFFFFFF;
    return randomSeed;
}
