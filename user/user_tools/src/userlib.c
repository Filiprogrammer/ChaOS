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

void puts(char* pString) {
    __asm__ volatile("int $0x7F"
                     :
                     : "a"(0), "b"(pString));
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
bool startsWith(char* str, char* prefix) {
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
    while (*dest)
        dest++;

    do {
        *dest++ = *src++;
    } while (*src);
    return dest;
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
char* trimStart(char* str) {
    // Trim leading space
    while (((unsigned char)*str) == 0x20) str++;
    return str;
}

/**
 * @brief Convert all letters of a string to lower case.
 * 
 * @param str string to be converted
 * @return char* pointer to the converted string str
 */
char* tolower(char* str) {
    while (*str != '\0') {
        if (*str >= 65 && *str <= 90)
            *str = *str + 32;

        ++str;
    }

    return str;
}

/**
 * @brief Find the first occurrence of substring in string.
 * 
 * @param string the string to be scanned
 * @param substring the string containing the sequence of characters to match
 * @return char* pointer to the first occurrence of substring in string
 */
char* strstr(char* string, char* substring) {
    char *a, *b;

    b = substring;
    if (*b == 0)
        return string;

    for (; *string != 0; ++string) {
        if (*string != *b)
            continue;

        a = string;
        while (1) {
            if (*b == 0)
                return string;

            if (*a++ != *b++)
                break;
        }
        b = substring;
    }
    return NULL;
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

int32_t power(int32_t base, int32_t n) {
    int32_t i, p;
    if (n == 0)
        return 1;
    p = 1;
    for (i = 1; i <= n; ++i)
        p = p * base;
    return p;
}

float powerf(float base, int32_t n) {
    int32_t i;
    float p;
    if (n == 0)
        return 1;
    p = 1;
    for (i = 1; i <= n; ++i)
        p = p * base;
    return p;
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

void itoa(int32_t value, char* valuestring) {
    int32_t min_flag;
    char swap, *p;
    min_flag = 0;

    if (0 > value) {
        *valuestring++ = '-';
        value = -INT_MAX > value ? min_flag = INT_MAX : -value;
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

void ftoa(float f, char* buffer) {
    if (f < 0)
        *(buffer++) = '-';
    int32_t i = (int32_t)f;
    itoa(i < 0 ? -i : i, buffer);

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

float sqrt(float x) {
    if (x < 0.0)
        return NAN;

    float result;
    __asm__("fsqrt"
            : "=t"(result)
            : "0"(x));
    return result;
}

uint32_t random() {
    randomSeed = (1780820173 * randomSeed + 718214708) % 0xFFFFFFFF;
    return randomSeed;
}
