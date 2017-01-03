/**
 * Kernel heap implementation
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <kernel/kernel.h>
#include <kernel/kernel_thread.h>
#include <kernel/bitset.h>
#include <mm/paging.h>

#include <mm/heap.h>

// Default kheap for kernel
kheap_t kheap_default;

void kheap_init(kheap_t *heap) {
    heap->first = 0;
}

/**
 * Install kheap as default kernel malloc
 */
void kheap_kernel_install() {
    kheap_init(&kheap_default);
}

/**
 * Debug function to print out the blocks in a kheap
 * @param heap kheap object to act on
 */
void _kheap_print(kheap_t *heap) {
    kheap_block_t *cur = heap->first;
    uint32_t i = 0;
    while (cur) {
        printk_debug("[kheap] block %u: Size: %u, Usable: %u\n", i++, cur->block_size,
                     cur->block_size - sizeof(kheap_block_t) - (cur->delimiters.length/32)*8);

        cur = cur->next;
    }
}

/**
 * Expand the heap to accommodate a block of the specified size
 * @param heap kheap object to act on
 * @param size minimum size of allocatable memory in block
 */
void kheap_expand(kheap_t *heap, size_t size) {
    // TODO: As block creation is costly, don't just allocate the bare
    // minimum. Some more advanced metric should be used to determine
    // the size of the new block.

    // Calculate number of pages required to meet this size
    uintptr_t pages_required = DIV_ROUND_UP(size, kpaging_data.page_size);
    // Make sure we have enough space for metadata too
    uint32_t bytes_per_int = SECTION_SIZE_DEFAULT * 32;
    uintptr_t block_size = pages_required * kpaging_data.page_size;
    uint32_t num_int_required = DIV_ROUND_UP(block_size, bytes_per_int); // Number of ints required for size
    uint32_t bitset_size = num_int_required * sizeof(uint32_t); // Size in memory of bitset

    while (pages_required*kpaging_data.page_size - sizeof(kheap_block_t) - bitset_size*2 < size) {
        // Recalculate the size of the bitset
        block_size = pages_required * kpaging_data.page_size;
        num_int_required = DIV_ROUND_UP(block_size, bytes_per_int); // Number of ints required for size
        bitset_size = num_int_required * sizeof(uint32_t); // Size in memory of bitset

        // Increase the number of pages
        ++pages_required;
    }

    // Allocate pages
    uintptr_t block_location = kpaging_data.highest_page+kpaging_data.page_size;
    //printk_debug("allocating %u pages at 0x%x", pages_required, block_location);
    uint32_t i;
    for (i=0; i<pages_required; i++) {
        kpaging_data.highest_page += kpaging_data.page_size;
        uintptr_t res = kpage_allocate(kpaging_data.highest_page, KPAGE_PRESENT | KPAGE_RW);
        ASSERT(res); // Make sure allocation succeeded
        //kernel_thread_sleep(5);
    }

    // Create block
    kheap_add_block(heap, (uintptr_t *)block_location, block_size, SECTION_SIZE_DEFAULT);
}

void kheap_add_block(kheap_t *heap, uintptr_t *start, uint32_t block_size,
                     uint32_t section_size) {
    uintptr_t addr = (uintptr_t)start;
    // Start address must be page aligned for now
    ASSERT(addr % 0x1000 == 0);

    // Sections must be large enough to store a 32-bit integer (4 bytes)
    ASSERT(section_size >= 4);

    // Get number of bytes required to store one bitset
    uint32_t bytes_per_int = section_size * 32; // Number of bytes represented in a single int
    uint32_t num_int_required = DIV_ROUND_UP(block_size, bytes_per_int); // Number of ints required for size
    uint32_t bitset_size = num_int_required * sizeof(uint32_t); // Size in memory of bitset
    uint32_t bitset_length = num_int_required * 32; // Number of entries in bitset

    // Get start address of used_sections bitset and delimiter bitset
    uintptr_t *used_bitset_start = (uintptr_t *)(addr + sizeof(kheap_block_t));
    uintptr_t *delimiter_bitset_start = (uintptr_t *)
                                        (addr + sizeof(kheap_block_t) + bitset_size);

    // Block must be big enough to store header and both bitsets
    ASSERT(block_size > sizeof(kheap_block_t) + (bitset_size*2));

    // Place block header at start of memory locaiton
    kheap_block_t *block = (kheap_block_t *)addr;
    block->next = NULL;
    block->block_size = block_size;
    block->section_size = section_size;
    block->start = addr;
    // Create used_sections bitset
    bitset_init(&block->used_sections, used_bitset_start, bitset_length);
    // Create delimiter bitset
    bitset_init(&block->delimiters, delimiter_bitset_start, bitset_length);

    // Mark first sections as reserved in used bitset (header + used bitset + delimiter bitset)
    uint32_t reserved = DIV_ROUND_UP((sizeof(kheap_block_t) + (bitset_size*2)),section_size);
    uint32_t i;
    for (i=0; i<reserved; i++) {
        bitset_set_bit(&block->used_sections, i);
    }

    // Add block to beginning of heap
    if (heap->first) {
        block->next = heap->first;
    }
    heap->first = block;
}

uintptr_t kheap_malloc(kheap_t *heap, size_t size) {
    // Iterate through heap linked list until we find a block with
    // enough contiguous sections to satisfy requested size
    kheap_block_t *cur = heap->first;

    while (cur) {
        //printf("Checking block at 0x%x\n", (uintptr_t)cur);
        // See if block is big enough
        uint32_t free_space = cur->block_size - sizeof(kheap_block_t) -
                                (cur->delimiters.length/32)*8;

        if (size > free_space) {
            goto skip_block;
        }

        // Calculate number of sections required
        uint32_t n_sec = DIV_ROUND_UP(size, cur->section_size);

        uint32_t i, j;
        for (i=0; i<cur->used_sections.length; i++) {
            if (!bitset_get_bit(&cur->used_sections, i)) {
                // Found empty section, see if we have n_sec-1 free after it
                bool found = true;
                for (j=i+1; j<i+n_sec; j++) {
                    if (j >= cur->used_sections.length ||
                        bitset_get_bit(&cur->used_sections, j)) {

                        // Not enough space
                        found = false;
                        break;
                    }
                }
                if (found) {
                    uintptr_t section_start = ((uintptr_t)cur)+(i * cur->section_size);

                    // Mark sections as allocated
                    for (j=i; j<i+n_sec; j++) {
                        bitset_set_bit(&cur->used_sections, j);
                    }

                    // Mark last section in delimiter bitset
                    bitset_set_bit(&cur->delimiters, i+n_sec-1);

                    // Return the starting address of the allocation
                    return section_start;
                }
            }
        }

    skip_block:
        cur = cur->next;
    }

    // We have searched the entire heap and found no free slots
    // Expand the heap and try again
    //printk_debug("Expanding the heap to accommodate %u more bytes", size);
    kheap_expand(heap, size);
    return kheap_malloc(heap, size);
}


void kheap_free(kheap_t *heap, uintptr_t addr) {
    // Walk through the block list until we find the block that contains
    // this allocation
    kheap_block_t *cur = heap->first;
    while (cur) {
        if (addr > cur->start && addr < cur->start + cur->block_size) {
            // Starting section number
            uint32_t sect_num = (addr - cur->start) / cur->section_size;

            // Go through delimiters bitset until we find the last section of
            // the allocation
            uint32_t i, last_section = 0;
            for (i=sect_num; i < cur->delimiters.length - sect_num; i++) {
                if (bitset_get_bit(&cur->delimiters, i)) {
                    // Found the delimiter, clear it and break
                    bitset_clear_bit(&cur->delimiters, i);
                    last_section = i;
                    break;
                }
            }

            // Make sure we found the delimiter
            // TODO: replace assert once debugging is done
            ASSERT(last_section);

            // Clear the sections in the used_sections bitset
            for (i=sect_num; i <= last_section; i++) {
                bitset_clear_bit(&cur->used_sections, i);
            }

            uint32_t allocation_size = (last_section - sect_num) + 1;

            // Clear memory
            // TODO: analyze performance penalty of doing this
            memset((void *)addr, 0, allocation_size);

            return;
        }

        cur = cur->next;
    }
}
