#include "string.h"

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
 * @brief Copy the string pointed to by src to dest.
 * 
 * @param dest pointer to destination where the content is to be copied to
 * @param src the string to be copied
 * @return char* pointer to the destination string
 */
char* strcpy(char* dest, const char* src) {
    char* tmp = dest;
    do {
        *dest++ = *src++;
    } while (*src);
    return tmp;
}

/**
 * @brief Copy up to n characters from the string pointed to, by src to dest. In a case where the length of src is less than n, the remainder of dest will be padded with null bytes.
 * 
 * @param dest pointer to destination where the content is to be copied to
 * @param src the string to be copied
 * @param n number of characters to be copied from src
 * @return char* pointer to the destination string
 */
char* strncpy(char* dest, const char* src, size_t n) {
    if (n != 0) {
        char* d = dest;
        const char* s = src;
        do {
            if ((*d++ = *s++) == 0) {
                /* NUL pad the remaining n-1 bytes */
                while (--n != 0)
                    *d++ = 0;
                break;
            }
        } while (--n != 0);
    }
    return (dest);
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
 * @brief Append the string pointed to by src to the end of the string pointed to by dest up to n characters long.
 * 
 * @param dest pointer to the destination array
 * @param src string to be appended
 * @param n maximum number of characters to be appended
 * @return char* 
 */
char* strncat(char* dest, const char* src, size_t n) {
    char* tmp = dest;

    if (n) {
        while (*dest)
            dest++;
        while ((*dest++ = *src++) != 0) {
            if (--n == 0) {
                *dest = '\0';
                break;
            }
        }
    }
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
 * @brief Convert all letters of a string to lower case.
 * 
 * @param str string to be converted
 * @return char* pointer to the converted string str
 */
char* tolower(char* str) {
    char* tmp = str;

    while (*str != '\0') {
        if (*str >= 65 && *str <= 90)
            *str = *str + 32;

        ++str;
    }

    return tmp;
}

/**
 * @brief Convert all letters of a string to upper case.
 * 
 * @param str string to be converted
 * @return char* pointer to the converted string str
 */
char* toupper(char* str) {
    char* tmp = str;

    while (*str != '\0') {
        if (*str >= 97 && *str <= 122)
            *str = *str - 32;

        ++str;
    }

    return tmp;
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

/**
 * @brief Locate the last occurrence of a character in a string.
 * 
 * @param str the string to search in
 * @param c the character to be located
 * @return int32_t the zero-based position of c if that character is found, or -1 if it is not.
 */
int32_t strrchr(char* str, char c) {
    int32_t ret = -1;

    for (int32_t i = 0; str[i] != 0; ++i)
        if (str[i] == c)
            ret = i;

    return ret;
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

/**
 * @brief Replace all instances of the passed substring in the passed string. (Make sure that the passed buffer containing the string is big enough.)
 * 
 * @param search the substring to look for
 * @param replace the substring with which to replace the found substrings
 * @param string the string in which to look
 * @return char* pointer to the passed string with the replacements performed
 */
char* stringReplace(char* search, char* replace, char* string) {
    char* searchStart;
    int len = 0;
    int searchLen = strlen(search);

    for (;;) {
        searchStart = strstr(string, search);
        if (searchStart == NULL)
            return string;

        len = searchStart - string;
        char tempString[strlen(string) - len - searchLen + 1];
        strcpy(tempString, searchStart + searchLen);

        char* i = replace;

        do {
            *searchStart++ = *i++;
        } while (*i);

        i = tempString;
        do {
            *searchStart++ = *i++;
        } while (*i);

        *searchStart = '\0';
    }
}

/**
 * @brief Compare at most the first n characters of two strings.
 * 
 * @param s1 first string to be compared
 * @param s2 second string to be compared
 * @param n maximum number of characters to be compared
 * @return int32_t less than 0 if s1 is less than s2, 0 if they are equal or 1 otherwise.
 */
int32_t strncmp(const char* s1, const char* s2, size_t n) {
    while (n--)
        if (*s1++ != *s2++)
            return *(unsigned char*)(s1 - 1) - *(unsigned char*)(s2 - 1);
    return 0;
}
