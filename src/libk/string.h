/**
 * @file string.h
 * @brief Standard C String and Memory Functions
 * 
 * Provides freestanding implementations of common string manipulation
 * and memory operation functions. These are required for kernel operation
 * since the kernel cannot link against the standard C library.
 * 
 * All functions follow standard C library semantics and behavior.
 * 
 * @note This is a freestanding implementation - no libc dependency
 */

#pragma once
#include <stddef.h>

/**
 * @brief Compare two null-terminated strings
 * 
 * @param s1 First string to compare
 * @param s2 Second string to compare
 * @return 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
int strcmp(const char *s1, const char *s2);

/**
 * @brief Compare up to n characters of two strings
 * 
 * @param s1 First string to compare
 * @param s2 Second string to compare
 * @param n Maximum number of characters to compare
 * @return 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
int strncmp(const char *s1, const char *s2, size_t n);

/**
 * @brief Calculate length of a null-terminated string
 * 
 * @param s String to measure
 * @return Number of characters before null terminator
 */
size_t strlen(const char *s);

/**
 * @brief Copy a null-terminated string
 * 
 * Copies src to dest including the null terminator.
 * 
 * @param dest Destination buffer (must be large enough)
 * @param src Source string to copy
 * @return Pointer to dest
 * @warning No bounds checking - ensure dest is large enough!
 */
char *strcpy(char *dest, const char *src);

/**
 * @brief Copy up to n characters of a string
 * 
 * Copies at most n characters from src to dest. If src is shorter than n,
 * dest is padded with null bytes. If src is n or longer, dest will NOT be
 * null-terminated.
 * 
 * @param dest Destination buffer
 * @param src Source string to copy
 * @param n Maximum number of characters to copy
 * @return Pointer to dest
 * @warning May not null-terminate if strlen(src) >= n
 */
char *strncpy(char *dest, const char *src, size_t n);

/**
 * @brief Copy memory from source to destination
 * 
 * Copies n bytes from src to dest. Regions must not overlap.
 * 
 * @param dest Destination pointer
 * @param src Source pointer
 * @param n Number of bytes to copy
 * @return Pointer to dest
 * @warning Undefined behavior if regions overlap - use memmove instead
 */
void *memcpy(void *dest, const void *src, size_t n);

/**
 * @brief Fill memory with a constant byte
 * 
 * Sets the first n bytes of dest to the value (converted to unsigned char).
 * 
 * @param dest Pointer to memory to fill
 * @param value Byte value to fill with (only lower 8 bits used)
 * @param n Number of bytes to fill
 * @return Pointer to dest
 */
void *memset(void *dest, int value, size_t n);

/**
 * @brief Compare two memory regions
 * 
 * Compares the first n bytes of s1 and s2.
 * 
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Number of bytes to compare
 * @return 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
int memcmp(const void *s1, const void *s2, size_t n);

/**
 * @brief Split string into tokens
 * 
 * Breaks a string into a sequence of tokens separated by any character
 * from delim. On first call, pass the string to tokenize. On subsequent
 * calls, pass NULL to continue tokenizing the same string.
 * 
 * @param str String to tokenize (or NULL to continue)
 * @param delim String containing delimiter characters
 * @return Pointer to next token, or NULL if no more tokens
 * @warning Not thread-safe! Modifies the input string!
 */
char *strtok(char *str, const char *delim);

/**
 * @brief Find first occurrence of character in string
 * 
 * Helper function for strtok.
 * 
 * @param s String to search
 * @param c Character to find
 * @return Pointer to first occurrence, or NULL if not found
 */
char *strchr(const char *s, int c);