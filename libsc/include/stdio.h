#ifndef _STDIO_H
#define _STDIO_H 1

#include <stdarg.h>

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

int vprintf(const char* restrict, va_list);
int sprintf(char *str, const char* __restrict, ...);
int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);

#ifdef __cplusplus
}
#endif

#endif
