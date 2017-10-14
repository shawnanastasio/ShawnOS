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

// Default kheap for kernel general allocations
kheap_t kheap_default;

// Critical kheap for kernel critical allocations (paging system, etc.)
kheap_t kheap_critical;

static inline bool __check_kheap_integrity(kheap_t *heap) {
    kheap_block_t *cur = heap->first;
    while (cur) {
        if (cur->magic != BLOCK_MAGIC || (cur->next && cur->next->magic != BLOCK_MAGIC)) {
            printk_debug("Wrong magic at: 0x%x", (uint32_t)cur);
            return false;
        }
        cur = cur->next;
    }
    return true;
}

void kheap_init(kheap_t *heap, uint32_t default_section_size, uint32_t flags, size_t max_expand_size) {
    heap->first = NULL;
    heap->default_section_size = default_section_size;
    heap->flags = flags;
    heap->effective_size = 0;
    heap->max_expand_size = max_expand_size;
    heap->cur_expand_size = 0;
}

/**
 * Install kheap as default kernel malloc
 */
void kheap_kalloc_install() {
    // Initalize the default heap
    // TODO: Use a better metric to determine the heap max expand size. Currently set to 1MB.
    // MUST NOT BE MORE THAN A SINGLE PAGE TABLE CAN HOLD OR THERE WILL NOT BE ENOUGH
    // CRITICAL SECTIONS FOR THE PAGING SYSTEM!!!!!!!
    size_t max_expand_size = 1024 * 1024;
    kheap_init(&kheap_default, SECTION_SIZE_DEFAULT, KHEAP_AUTO_EXPAND, max_expand_size);
    kheap_init(&kheap_critical, kpaging_data.page_size, KHEAP_ALIGN, max_expand_size);
    // Initalize the critical heap with a single block of page size
    ASSERT(kheap_expand(&kheap_critical, kpaging_data.page_size) == K_SUCCESS);

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
    // If this heap has KHEAP_ALIGN set, increase the size by section_size
    if (heap->flags & KHEAP_ALIGN) {
        size += heap->default_section_size;     
    }

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

    // Allocate pages
    uintptr_t block_location = kpaging_data.highest_page+kpaging_data.page_size;
    uintptr_t cur_location = kpaging_data.highest_page;
    kpaging_data.highest_page += pages_required * kpaging_data.page_size;
    
    if ((uintptr_t)heap == (uintptr_t)&kheap_critical) {
        printk_debug("newcritb from: 0x%x to 0x%x", cur_location, kpaging_data.highest_page);
    } else {
        printk_debug("newblock from: 0x%x to 0x%x", cur_location, kpaging_data.highest_page);
    }

    uint32_t i;
    for (i=0; i<=pages_required; i++) {
        uintptr_t res = kpage_allocate(cur_location, KPAGE_PRESENT | KPAGE_RW);
        ASSERT(__check_kheap_integrity(heap));
        if (!res) {
            // Allocation failed, free all previously allocated pages and return
            for (; i>=pages_required; i--) {
                kpage_free(cur_location);
                cur_location -= kpaging_data.page_size;
            }
            return -K_OOM;
        }
        cur_location += kpaging_data.page_size;
    }

    // If KHEAP_ALIGN is set, align the starting block location
    if (heap->flags & KHEAP_ALIGN) {
        block_location += block_location % heap->default_section_size;
    }

    // Create block
    kheap_add_block(heap, (uintptr_t)block_location, block_size, heap->default_section_size);
    return K_SUCCESS;
}

void kheap_add_block(kheap_t *heap, uintptr_t addr, size_t block_size,
                     uint32_t section_size) {
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
    block->magic = BLOCK_MAGIC;
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

    // Increase heap's effective size
    heap->effective_size += block_size - sizeof(kheap_block_t) + (bitset_size * 2);

    // Add block to beginning of heap
    block->next = heap->first;
    heap->first = block;
}

/**
 * Kernel alloc interface function to allocate memory
 * Redirects to kheap_malloc
 */
uintptr_t __kheap_kalloc_malloc_real(size_t size, uintptr_t *phys, uint32_t flags) {
    uintptr_t res;
    kheap_t *heap;

    if (flags & KALLOC_CRITICAL) {
        heap = &kheap_critical;

        // If the critical heap is at >=90% usage, we should increase it by 
        // pagesize * 128 bytes.
        printk_debug("TODO: Expand critical heap");

    } else {
        heap = &kheap_default;
    }

    // Handle allocation
    if (flags & KALLOC_PAGE_ALIGN) {
        res = kheap_malloc(heap, size, kpaging_data.page_size, flags);
    } else {
        res = kheap_malloc(heap, size, 0, flags);
    }

    // Handle physical address output
    if (phys) {
        *phys = kpage_get_phys(res);
    }

    return res;
}

/**
 * Kernel alloc interface function to free allocated memory
 * Redirects to kheap_free
 */
void __kheap_kalloc_free(uintptr_t addr) {
    kheap_free(&kheap_default, addr);
}

uintptr_t kheap_malloc(kheap_t *heap, size_t size, size_t align, uint32_t flags) {
    ASSERT(__check_kheap_integrity(heap));
    size_t original_size = size;
    bool manual_align = false;
    // Handle alignment
    if (align > 0) {
        // If KHEAP_ALIGN isn't enabled or the given amount isn't equal to the section size,
        // manually align the allocation.
        if ((heap->flags & KHEAP_ALIGN) == 0 || heap->default_section_size != align) {
            size += align;
            manual_align = true;
        }
    }
    
    // Iterate through heap linked list until we find a block with
    // enough contiguous sections to satisfy requested size
    kheap_block_t *cur = heap->first;
    
    uint32_t block_no_dbg;
    for (block_no_dbg = 0; cur; block_no_dbg++) {
        //printf("Checking block at 0x%x\n", (uintptr_t)cur);
        // See if block is big enough
        uint32_t free_space = cur->block_size - sizeof(kheap_block_t) -
                                (cur->delimiters.length/32)*8;

        // Calculate number of sections required
        uint32_t n_sec = DIV_ROUND_UP(size, cur->section_size);


        if (size > free_space || n_sec > cur->free_sections) {
            goto skip_block;
        }

        
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

                    // Mark first and last section in delimiter bitset
                    bitset_set_bit(&cur->delimiters, i);
                    bitset_set_bit(&cur->delimiters, i+n_sec-1);

                    // Align the address if requested
                    if (manual_align && section_start % align > 0) {
                        section_start += align - (section_start % align);
                    }
                    
                    // Decrease this block's free section count
                    cur->free_sections -= n_sec;

                    // Return the starting address of the allocation
                    return section_start;
                }
            }
        }

    skip_block:
        if (cur->next && cur->next->magic != BLOCK_MAGIC) {
            printk_debug("Next block 0x%x has invalid magic: 0x%x", (uint32_t)cur->next, cur->next->magic);
            printk_debug("next u32: 0x%x", (uint32_t)(((uint32_t *)cur->next->magic)+2));
            PANIC("");
        }
        cur = cur->next;
    }

    // Not enough memory in heap, if KHEAP_AUTO_EXPAND is set, try expanding the heap
    if ((heap->flags & KHEAP_AUTO_EXPAND) == 0) {
        return 0;
    }

    if (heap->cur_expand_size < heap->max_expand_size) {
        // The heap's current expand size can be increased
        // Use the current heap size + the size of the new allocation
        // as the new expand size unless it's too big
        if (heap->effective_size + size < heap->max_expand_size) {
            heap->cur_expand_size = heap->effective_size + size;
        } else {
            heap->cur_expand_size = heap->max_expand_size;
        }
    }

    //printk_debug("expanding heap by %u bytes", heap->cur_expand_size);
    //kernel_thread_sleep(2);

    k_return_t res = kheap_expand(heap, heap->cur_expand_size);
    if (res >= 0) {
        return kheap_malloc(heap, original_size, align, flags);
    } else {
        return 0;
    }
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

            // Go through delimiters bitset until we find the
            // the first and last section of the allocation
            uint32_t i, first_section = 0, last_section = 0;
            
            // First section
            for (i=sect_num; i > 0; i--) {
                if (bitset_get_bit(&cur->delimiters, i)) {
                    // Found the first delimiter, clear it and break
                    bitset_clear_bit(&cur->delimiters, i);
                    first_section = i;
                    break;
                }
            }
            
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
            ASSERT(first_section);
            ASSERT(last_section);

            // Clear the sections in the used_sections bitset
            for (i=first_section; i <= last_section; i++) {
                bitset_clear_bit(&cur->used_sections, i);
            }

            //printk_debug("Cleared %u sections", last_section-first_section+1);
            return true;
        }

        cur = cur->next;
    }
    return false;
}
