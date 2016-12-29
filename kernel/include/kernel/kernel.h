#pragma once

#include <stdlib.h>

#define ASSERT(x,err)          \
    do {                       \
        if (!(x)) {              \
            printk_debug(err); \
            abort();           \
        }                      \
    } while (0)                \

void kernel_task();
void printk_debug(char *string);
