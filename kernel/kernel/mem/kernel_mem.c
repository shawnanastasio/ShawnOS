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
void kernel_mem_mmap_read(uint32_t mmap_length, multiboot_memory_map_t *mmap) {
    printk_debug("Parsing multiboot memory map.");

    uintptr_t cur_mmap_addr = (uintptr_t)mmap;
    uintptr_t mmap_end_addr = cur_mmap_addr + mmap_length;
    while (cur_mmap_addr < mmap_end_addr) {
        multiboot_memory_map_t *current_entry = (multiboot_memory_map_t *)cur_mmap_addr;

        // Print out information for this entry
        printf("entry_location: 0x%lx entry_size: 0x%lx addr: 0x%lx len: 0x%lx\n", cur_mmap_addr, current_entry->size, current_entry->addr, current_entry->len);

        // Increment by the size to get to the next entry
        cur_mmap_addr += current_entry->size + sizeof(uintptr_t);
    }
}
