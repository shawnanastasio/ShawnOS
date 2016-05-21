/**
 * Kernel Memory Allocation functions
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <kernel/kernel.h>
#include <kernel/kernel_thread.h>

#include <arch/i386/multiboot.h>

/**
 * Read multiboot memory map and determine free memory chunks
 */
void kernel_mem_mmap_read(mboot_memmap_t *mmap) {
    uint64_t mmap_length = mmap->size;
    printf("Initalizing memory map length: %d\n", (int)mmap_length);

    printk_debug("Parsing multiboot memory map");
    /*
    uintptr_t cur_mmap = (uintptr_t)mmap;

    uint16_t i = 1;
    while (cur_mmap < (uintptr_t)mmap + mmap_length) {
        printf("Found memory chunk: %d\n", i++);
        cur_mmap = cur_mmap + mmap->size + sizeof(uintptr_t);
    }
    */
}
