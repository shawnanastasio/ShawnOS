/**
 * Kernel Memory Allocation functions
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <kernel/mem/kernel_mem.h>
#include <kernel/kernel_thread.h>

#include <arch/i386/mem.h>

/**
 * Structure that contains references to all architecture-specific functions and
 * variables. Must be initalized by architecture code. See kernel_mem.h
 */
kernel_mem_allocator_data_t kernel_mem_info;

/**
 * Allocates a frame and returns the frame number
 */
uint32_t kernel_mem_allocate_frame() {
    return kernel_mem_info.allocate_frame();
}

/**
 * Gets a frame's start address from a frame number
 */
uint32_t kernel_mem_get_frame_start_addr(uint32_t num) {
    return kernel_mem_info.get_frame_start_addr(num);
}

/**
 * Gets a frame's number from any address in it.
 */
uint32_t kernel_mem_get_frame_num(uint32_t addr) {
    return kernel_mem_info.get_frame_num(addr);
}

/**
 * Allocate raw memory before paging is activated
 * @param size size in bytes of memory chunk
 */
uint32_t kernel_mem_kmalloc(uint32_t size) {
    return kernel_mem_info.kmalloc(size);
}
