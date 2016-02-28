#ifndef _LIBSK_H
#define _LIBSK_H 1
/**
 * Libc functions
 * I really don't need to comment most of these
 */

size_t strlen(const char* str);

int memcmp(const void* s1, const void* s2,size_t n);

void *memcpy(void *str1, const void *str2, size_t n);

void *memmove(void *dest, const void *src, size_t n);

void *memset(void *s, int c, size_t n);

#endif
