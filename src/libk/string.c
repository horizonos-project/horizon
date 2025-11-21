#include "string.h"

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0)
        return 0;
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

size_t strlen(const char *s) {
    const char *p = s;
    while (*p)
        p++;
    return (size_t)(p - s);
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++))
        ;
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    for (size_t i = 0; i < n; i++)
        d[i] = s[i];
    return dest;
}

void *memset(void *dest, int value, size_t n) {
    unsigned char *d = dest;
    for (size_t i = 0; i < n; i++)
        d[i] = (unsigned char)value;
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *a = s1, *b = s2;
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i])
            return a[i] - b[i];
    }
    return 0;
}

char *strtok(char *str, const char *delim) {
    static char *saved = NULL;  // Remember position between calls
    
    // If str is NULL, continue from saved position
    if (str == NULL) {
        str = saved;
    }
    
    // If nothing left to tokenize, return NULL
    if (str == NULL) {
        return NULL;
    }
    
    // Skip leading delimiters
    while (*str && strchr(delim, *str)) {
        str++;
    }
    
    // If we've reached the end, no more tokens
    if (*str == '\0') {
        saved = NULL;
        return NULL;
    }
    
    // Found start of token
    char *token_start = str;
    
    // Find end of token (next delimiter or end of string)
    while (*str && !strchr(delim, *str)) {
        str++;
    }
    
    // If we found a delimiter, replace it with null terminator
    if (*str) {
        *str = '\0';
        saved = str + 1;
    } else {
        saved = NULL;  // No more tokens after this
    }
    
    return token_start;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) {
            return (char*)s;
        }
        s++;
    }
    
    // Check if we're looking for null terminator
    if (c == '\0') {
        return (char*)s;
    }
    
    return NULL;
}