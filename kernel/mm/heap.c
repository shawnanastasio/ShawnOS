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
#include <mm/alloc.h>
#include <mm/heap.h>
#include <mm/asa.h>

// Default kheap for kernel general allocations
kheap_t kheap_default;

static inline bool __check_kheap_integrity(kheap_t *heap) {
#ifdef KHEAP_DEBUG
    kheap_block_t *cur = heap->first;
    while (cur) {
        if (cur->magic != BLOCK_MAGIC || (cur->next && cur->next->magic != BLOCK_MAGIC)) {
            printk_debug("Wrong magic at: 0x%x", (uint32_t)cur);
            return false;
        }
        cur = cur->next;
    }
    return true;
#else
    heap = heap;
    return true;
#endif
}

void kheap_init(kheap_t *heap, uint32_t default_section_size, uint32_t min_block_size,
                uint32_t flags) {
    heap->first = NULL;
    heap->default_section_size = default_section_size;
    heap->min_block_size = min_block_size;
    heap->flags = flags;
    heap->effective_size = 0;
    heap->total_free_sections = 0;
}

/**
 * Install kheap as default kernel malloc
 */
void kheap_kalloc_install() {
    // Initalize the default heap
    kheap_init(&kheap_default, SECTION_SIZE_DEFAULT, MIN_BLOCK_SIZE_DEFAULT,
               KHEAP_AUTO_EXPAND);

    kalloc_data.kalloc_malloc_real = __kheap_kalloc_malloc_real;
    kalloc_data.kalloc_free = __kheap_kalloc_free;
}


/**
 * Debug function to print out the blocks in a kheap
 * @param heap kheap object to act on
 */
static inline void _kheap_print(kheap_t *heap) {
    kheap_block_t *cur = heap->first;
    uint32_t i = 0;
    while (cur) {
        printk_debug("[kheap] block %u: Size: %u, Usable: %u", i++, cur->block_size,
                     cur->block_size - sizeof(kheap_block_t) - (cur->delimiters.length/32)*8);

        cur = cur->next;
    }
}

/**
 * Expand the heap to accommodate a block of the specified size
 * @param heap kheap object to act on
 * @param size minimum size of allocatable memory in block
 */
k_return_t kheap_expand(kheap_t *heap, size_t size) {
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
    block_size = pages_required * kpaging_data.page_size;

    // Allocate virtual pages from ASA
    uintptr_t block_location = (uintptr_t)asa_alloc(pages_required);
    if (!block_location) {
        printk_debug("ASA Alloc failed!");
        return K_OOM;
    }

    // Map virtual pages
    k_return_t ret;
    uint32_t i;
    for (i=0; i<pages_required; i++) {
        ret = kpage_allocate(block_location + (i * kpaging_data.page_size), KPAGE_PRESENT | KPAGE_RW);
        if (K_FAILED(ret)) {
            // Allocation failed, free all previously allocated pages and return
            asa_free((void *)block_location, pages_required);
            while (i-- > 0) {
                kpage_free(block_location + (i * kpaging_data.page_size));
            }
            printk_debug("map failed!");
            return K_OOM;
        }
    }

    // Create block
    kheap_add_block(heap, (uintptr_t)block_location, block_size, heap->default_section_size);
    return K_SUCCESS;
}

void kheap_add_block(kheap_t *heap, uintptr_t addr, size_t block_size, uint32_t section_size) {
    // Start address must be page aligned for now
    ASSERT(addr % 0x1000 == 0);

    // Sections must be large enough to store a 32-bit integer (4 bytes)
    ASSERT(section_size >= 4);

    // Get number of bytes required to store one bitset
    uint32_t bytes_per_int = section_size * 32; // Number of bytes represented in a single int
    uint32_t num_int_required = DIV_ROUND_UP(block_size, bytes_per_int); // Number of ints required for size
    uint32_t bitset_size = num_int_required * sizeof(uint32_t); // Size in memory of bitset
    uint32_t bitset_length = DIV_ROUND_UP(block_size, section_size);

    // Get start address of used_sections bitset and delimiter bitset
    void *used_bitset_start = (void *)(addr + sizeof(kheap_block_t));
    void *delimiter_bitset_start = (void *)
                                        (addr + sizeof(kheap_block_t) + bitset_size);

    // Block must be big enough to store header and both bitsets
    ASSERT(block_size > sizeof(kheap_block_t) + (bitset_size*2));

    // Place block header at start of memory locaiton
    kheap_block_t *block = (kheap_block_t *)addr;
    block->next = NULL;
    block->block_size = block_size;
    block->section_size = section_size;
    block->start = addr;
#ifdef KHEAP_DEBUG
    block->magic = BLOCK_MAGIC;
#endif
    // Create used_sections bitset
    bitset_init(&block->used_sections, used_bitset_start, bitset_length);
    // Create delimiter bitset
    bitset_init(&block->delimiters, delimiter_bitset_start, bitset_length);

    // Mark first sections as reserved in used bitset
    // (header + used bitset + delimiter bitset)
    uint32_t reserved = DIV_ROUND_UP((sizeof(kheap_block_t) + (bitset_size*2)), section_size);
    uint32_t i;
    for (i=0; i<reserved; i++) {
        bitset_set_bit(&block->used_sections, i);
    }
    block->free_sections = bitset_length - reserved;
    block->first_free_section = reserved;

    heap->total_free_sections += bitset_length - reserved;

    // Increase heap's effective size
    heap->effective_size += block_size - sizeof(kheap_block_t) + (bitset_size * 2);

    // Add block to beginning of heap
    block->next = heap->first;
    heap->first = block;
}

/*
 * Wrapper functions to conform to standard interfaces
 */
uintptr_t __kheap_kalloc_malloc_real(size_t size, uintptr_t *phys, uint32_t flags) {
    k_return_t ret;
    uintptr_t res;

    ret = kheap_malloc(&kheap_default, size,
                       (flags & KALLOC_PAGE_ALIGN) ? kpaging_data.page_size : 0, &res);

    if (K_FAILED(ret)) {
        PANIC("kheap OOM!");
    }

    if (phys) {
        *phys = kpage_get_phys(res);
    }

    return res;
}

void __kheap_kalloc_free(uintptr_t addr) {
    kheap_free(&kheap_default, addr);
}

k_return_t kheap_malloc(kheap_t *heap, size_t size, size_t align, uintptr_t *out) {
#ifdef KHEAP_DEBUG
    ASSERT(__check_kheap_integrity(heap));
#endif

    bool manual_align = false;
    k_return_t ret;

    // Handle alignment
    if (align > 0) {
        // If the given amount isn't equal to the section size, manually align the allocation.
        if (heap->default_section_size != align) {
            size += align;
            manual_align = true;
        }
    }

    // Iterate through heap linked list until we find a block with
    // enough contiguous sections to satisfy requested size
    kheap_block_t *cur = heap->first;

    // If the heap is empty, add an initial block
    if (!cur) {
        ret = kheap_expand(heap, MAX(heap->min_block_size, size));
        if (K_FAILED(ret)) {
            return ret;
        }
        cur = heap->first;
    }

    uint32_t block_no_dbg;
    for (block_no_dbg = 0; cur; block_no_dbg++) {
        //printf("Checking block at 0x%x\n", (uintptr_t)cur);
        // See if block is big enough
        uint32_t max_free_space = cur->block_size - sizeof(kheap_block_t) -
                              (cur->delimiters.length / 32) * 4 - (cur->used_sections.length / 32) * 4;

        // Calculate number of sections required
        uint32_t n_sec = DIV_ROUND_UP(size, cur->section_size);
        if (size > max_free_space || n_sec > cur->free_sections) {
            goto skip_block;
        }

        uint32_t i, j, k;
        uint32_t first_free = 0;
        uint32_t continuous_pages = 0;
        for (k=0; k<cur->used_sections.length; k++) {
            // Calculate the actual index based off of the first free section
            i = (cur->first_free_section + k) % cur->used_sections.length;
            if (!bitset_get_bit(&cur->used_sections, i)) {
                if (i < first_free) {
                    // Wrapped around, unset continuous flag
                    continuous_pages = 0;
                } else if (continuous_pages == 0) {
                    first_free = i;
                }

                ++continuous_pages;

                // Check if we have accumulated n_sec free pages
                if (continuous_pages == n_sec) {
                    uintptr_t section_start = ((uintptr_t)cur)+(first_free * cur->section_size);
                    // Mark sections as allocated
                    uint32_t end = first_free + n_sec;
                    for (j=first_free; j<first_free+n_sec;) {
                        if (j % 32 == 0 && j + 32 < end) {
                            cur->used_sections.start[j / 32] = 0xFFFFFFFF;
                            j += 32;
                        } else {
                            bitset_set_bit(&cur->used_sections, j);
                            ++j;
                        }
                    }

                    // Mark last section in delimiter bitset
                    bitset_set_bit(&cur->delimiters, first_free+n_sec-1);

                    // Align the address if requested
                    if (manual_align && section_start % align > 0) {
                        section_start += align - (section_start % align);
                    }

                    // Decrease this block's free section count
                    cur->free_sections -= n_sec;
                    heap->total_free_sections -= n_sec;

                    // Set the first free section to the end of the allocation
                    cur->first_free_section = first_free + n_sec;

                    // Return the starting address of the allocation
                    *out = section_start;
                    return K_SUCCESS;
                }
            } else {
                continuous_pages = 0;
            }
        }

    skip_block:
#ifdef KHEAP_DEBUG
        if (cur->next && cur->next->magic != BLOCK_MAGIC) {
            printk_debug("Next block 0x%x has invalid magic: 0x%x", (uint32_t)cur->next, cur->next->magic);
            PANIC("");
        }
#endif
        cur = cur->next;

        if (!cur) {
            // Reached end of heap, expand it
            ret = kheap_expand(heap, MAX(heap->min_block_size, size));
            if (K_FAILED(ret)) {
                return ret;
            }
            cur = heap->first;
        }
    }

    // If we got here, there's not enough memory in the heap to satisfy the allocation
    return K_OOM;
}


/**
 * Free an allocation in the given heap
 * @param heap kernel heap to act on
 * @param addr address to free
 * @return did allocation exist and get freed?
 */
bool kheap_free(kheap_t *heap, uintptr_t addr) {
    // Walk through the block list until we find the block that contains
    // this allocation
    kheap_block_t *cur = heap->first;
    while (cur) {
        if (addr > cur->start && addr < cur->start + cur->block_size) {
            // Provided section number
            uint32_t sect_num = (addr - cur->start) / cur->section_size;

            // Make sure the given address is actually allocated
            ASSERT(bitset_get_bit(&cur->used_sections, sect_num));

            // Go through delimiters bitset until we find the
            // the last section of the allocation
            uint32_t i, last_section = 0;

            // Last section
            for (i=sect_num; i < cur->delimiters.length; i++) {
                if (bitset_get_bit(&cur->delimiters, i)) {
                    // Found the delimiter, clear it and break
                    bitset_clear_bit(&cur->delimiters, i);
                    last_section = i;
                    break;
                }
            }

            // Make sure we found the delimiters
            // TODO: replace assert once debugging is done
            ASSERT(last_section);

            // Clear the sections in the used_sections bitset
            for (i=sect_num; i <= last_section; i++) {
                bitset_clear_bit(&cur->used_sections, i);
            }

            // Update the first free section for this block
            cur->first_free_section = sect_num;

            // Update heap/block metadata
            cur->free_sections += last_section - sect_num + 1;
            heap->total_free_sections += last_section - sect_num + 1;
            return true;
        }

        cur = cur->next;
    }
    return false;
}
