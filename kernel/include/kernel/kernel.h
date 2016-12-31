#pragma once

#include <stdlib.h>

// Macros

#define DIV_ROUND_UP(a,b) (((a - 1) / b) + 1)

#define ASSERT(x)              \
    do {                       \
        if (!(x)) {            \
            printk_debug("ASSERT failed: %s at %s:%d", #x, __FILE__, __LINE__);\
            abort();           \
        }                      \
    } while (0)                \


void kernel_task();
void printk_debug(char *fmt, ...);
