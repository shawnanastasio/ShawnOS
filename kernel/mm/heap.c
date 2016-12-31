/**
 * Kernel heap implementation
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <kernel/kernel.h>
#include <kernel/bitset.h>

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

// TODO: Use abstractions for architecture-specific paging code
void kheap_add_block(kheap_t *heap, uintptr_t *start, uint32_t block_size,
                     uint32_t section_size) {
    uintptr_t addr = (uintptr_t)start;
    // Start address must be page aligned for now
    ASSERT(addr % 0x1000 == 0);

    // Sections must be large enough to store kheap_alloc_header
    ASSERT(section_size > sizeof(kheap_alloc_header_t));

    // Get number of bytes required to store bitset
    uint32_t bytes_per_int = section_size * 32; // Number of bytes represented in a single int
    uint32_t num_int_required = DIV_ROUND_UP(block_size, bytes_per_int); // Number of ints required for size
    uint32_t bitset_size = num_int_required * sizeof(uint32_t);

    // Get start address of bitset
    uintptr_t *bitset_start = (uintptr_t *)(addr + sizeof(kheap_block_t));

    // Block must be big enough to store header and bitset
    ASSERT(block_size > sizeof(kheap_block_t) + bitset_size);

    // Place block header at start of memory locaiton
    kheap_block_t *block = (kheap_block_t *)addr;
    block->next = NULL;
    block->block_size = block_size;
    block->section_size = section_size;
    block->start = addr;
    // Create bitset
    bitset_init(&block->bitset, bitset_start, bitset_size);

    // Mark first sections as reserved in bitset (header + bitset itself)
    uint32_t reserved = DIV_ROUND_UP((sizeof(kheap_block_t) + bitset_size),section_size);
    uint32_t i;
    for (i=0; i<reserved; i++) {
        bitset_set_bit(&block->bitset, i);
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

    // Real size is requested size + sizeof(kheap_alloc_header_t)
    size_t real_size = size + sizeof(kheap_alloc_header_t);

    while (cur) {
        //printf("Checking block at 0x%x\n", (uintptr_t)cur);
        // See if block is big enough
        uint32_t free_space = cur->block_size - sizeof(kheap_block_t) - cur->bitset.size;
        if (size > free_space)
            goto skip_block;

        // Calculate number of sections required
        uint32_t n_sec = DIV_ROUND_UP(real_size, cur->section_size);

        uint32_t i, j;
        for (i=0; i<cur->bitset.size; i++) {
            if (!bitset_get_bit(&cur->bitset, i)) {
                // Found empty section, see if we have n_sec-1 free after it
                bool found = true;
                for (j=i+1; j<i+n_sec; j++) {
                    if (j > cur->bitset.size || bitset_get_bit(&cur->bitset, j)) {
                        // Not enough space
                        found = false;
                        break;
                    }
                }
                if (found) {
                    // Place allocation header at beginning of section
                    uintptr_t section_start = ((uintptr_t)cur)+(i * cur->section_size);
                    kheap_alloc_header_t *alloc_header = (kheap_alloc_header_t *)section_start;
                    alloc_header->magic = SECTION_MAGIC;
                    alloc_header->size = real_size;

                    // Mark sections as allocated
                    for (j=i; j<i+n_sec; j++) {
                        bitset_set_bit(&cur->bitset, j);
                    }

                    // Return the starting address of the user-accessible memory
                    return section_start + sizeof(kheap_alloc_header_t);
                }
            }
        }

    skip_block:
        cur = cur->next;
    }

    // We have searched the entire heap and found no free slots
    return 0;
}


void kheap_free(kheap_t *heap, uintptr_t addr) {
    // Make sure our allocation header exists before the given address
    kheap_alloc_header_t *header = (kheap_alloc_header_t *)
                                    (addr - sizeof(kheap_alloc_header_t));

    // Validate the magic number
    ASSERT(header->magic == SECTION_MAGIC);

    // The address that the allocation actually starts at (including header)
    uint32_t real_addr = addr - sizeof(kheap_alloc_header_t);

    // Walk through the block list until we find the block that contains
    // this allocation
    kheap_block_t *cur = heap->first;
    while (cur) {
        if (real_addr > cur->start && real_addr < cur->start + cur->block_size) {
            // Starting section number
            uint32_t sect_num = (real_addr - cur->start) / cur->section_size;
            // Number of sections allocated
            uint32_t n_sec = DIV_ROUND_UP(header->size, cur->section_size);

            // Mark sections free
            uint32_t i;
            for (i=sect_num; i<sect_num+n_sec; i++) {
                bitset_clear_bit(&cur->bitset, i);
            }

            // Clear section ?
            memset((void *)real_addr, 0, header->size);
        }

        cur = cur->next;
    }
}
