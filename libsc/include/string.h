#ifndef _STRING_H
#define _STRING_H 1

#include <sys/cdefs.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
char *strcpy(char * restrict dest, const char * restrict src);
char *strncpy(char *restrict dest, const char *restrict src, size_t n);
char *strcat(char *dest, const char *src);
int strcmp(const char *a, const char *b);

#ifdef __cplusplus
}
#endif

#endif
