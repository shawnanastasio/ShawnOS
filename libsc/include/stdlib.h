#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

void itoa(int n, char s[]);

__attribute__((__noreturn__))
void abort(void);

#ifdef __cplusplus
}
#endif

#endif
