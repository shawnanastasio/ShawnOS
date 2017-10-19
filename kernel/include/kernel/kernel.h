#pragma once

#include <stdint.h>
#include <stdlib.h>

/**
 * Kernel result codes
 * Errors should be negative.
 */
typedef int32_t k_return_t;
#define K_SUCCESS 0    // Success
#define K_UNIMPL  1    // Unimplemented
#define K_OOM     2    // Out of memory
#define K_IO      3    // Hardware I/O error occurred
#define K_NOSPACE 4    // Not enough storage space
#define K_NOTSUP  5    // Operation not supported
#define K_INVALOP 6    // Invalid operation

// Macro to determine if kernel return code is a failure
#define K_FAILED(code) (((code) < 0))


/**
 * Spinlock macros
 * See: wiki.osdev.org/Spinlock
 */
#define SPINLOCK_DECLARE(name) volatile int name ## Locked
#define SPINLOCK_LOCK(name) \
	while (!__sync_bool_compare_and_swap(& name ## Locked, 0, 1)); \
	__sync_synchronize();
#define SPINLOCK_UNLOCK(name) \
	__sync_synchronize(); \
	name ## Locked = 0;


/**
 * Math macros
 */
#define DIV_ROUND_UP(a,b) ((((a) - 1) / (b)) + 1)


/**
 * Kernel debug macros
 */
#define ASSERT(x)              \
    do {                       \
        if (!(x)) {            \
            printk_debug("ASSERT failed: %s at %s:%d", #x, __FILE__, __LINE__);\
            abort();           \
        }                      \
    } while (0)                

#define PANIC(reason)                   \
    do {                                \
        printk_debug("PANIC: %s at %s:%d\n", reason, __FILE__, __LINE__); \
        abort();                        \
    } while (0)                     

void kernel_task();
void printk_debug(char *fmt, ...);
