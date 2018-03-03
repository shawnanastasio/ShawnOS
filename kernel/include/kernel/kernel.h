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
#define K_FAILED(code) (((code) != K_SUCCESS))


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

// "Beautiful" C11 generic MAX() macro implementation
#define max_impl_custom(T, name) \
        static inline T __max_impl_ ## name  \
        (T a, T b) {                         \
            return (a>b)?a:b;                \
        }                                    \

#define max_impl(T) max_impl_custom(T, T)

max_impl(char)
max_impl(short)
max_impl(int)
max_impl(long)
max_impl_custom(long long, longlong)
max_impl_custom(unsigned char, uchar)
max_impl_custom(unsigned short, ushort)
max_impl_custom(unsigned int, uint)
max_impl_custom(unsigned long, ulong)
max_impl_custom(unsigned long long, ulonglong)

#define MAX(a, b) _Generic((a),              \
    char: __max_impl_char,                   \
    short: __max_impl_short,                 \
    int: __max_impl_int,                     \
    long: __max_impl_long,                   \
    long long: __max_impl_longlong,          \
    unsigned char: __max_impl_uchar,         \
    unsigned short: __max_impl_ushort,       \
    unsigned int: __max_impl_uint,           \
    unsigned long: __max_impl_ulong,         \
    unsigned long long: __max_impl_ulonglong \
)((a), (b))


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

/**
 * Architecture-specific constants
 * TODO: ifdef i386
 */

/* Memory up to this address is reserved for early init and will be identity mapped */
#define KVIRT_RESERVED 0x179000

/* Maximum kernel virtual address */
#define KVIRT_MAX 0x3FFFFFFF


void kernel_task();
void printk_debug(char *fmt, ...);
