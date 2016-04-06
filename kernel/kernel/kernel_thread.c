/**
 * Thread-related functions for ShawnOS kernel
 *
 * Normally these functions should be called via libc
 */

#include <stdint.h>

/* Architecture specific includes */
#include <drivers/pc/pit.h>

void kernel_thread_sleep(uint32_t seconds) {
    // i386 implementation
    pit_timer_wait(seconds);
}
