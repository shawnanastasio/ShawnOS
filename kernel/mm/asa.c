/**
 * Kernel address space allocator
 *
 * Provides functions for reserving virtual pages in the kernel address space.
 * Must be mapped to kernel page tables before use.
 */

#include <stdint.h>

#include <kernel/kernel.h>
#include <kernel/bitset.h>
#include <mm/asa.h>
#include <mm/alloc.h>
#include <mm/paging.h>

kasa_data_t kasa_data;

/**
 * Initalize the address space allocator.
 * Will allocate no less than KVIRT_MAX/PAGE_SIZE/32 * 4 bytes of memory from kmalloc()
 */
k_return_t asa_init(uint32_t page_size) {
    kasa_data.page_size = page_size;

    uint32_t n_entries = KVIRT_MAX/kasa_data.page_size;
    uint32_t size = DIV_ROUND_UP(n_entries, 32) * sizeof(uint32_t);

    // Allocate bitset
    void *bitset_start = kmalloc(size, KALLOC_CRITICAL);
    if (!bitset_start) return K_OOM;
    bitset_init(&kasa_data.pages, bitset_start, n_entries);

    // Mark all pages from 0 up to kpaging_data.highest_page as used
    uint32_t i;
    uint32_t max = KVIRT_RESERVED / kasa_data.page_size;
    for (i=0; i<max; i++) {
        bitset_set_bit(&kasa_data.pages, i);
    }

    return K_SUCCESS;
}

/**
 * Allocate n pages from the kernel virtual address space
 */
void *asa_alloc(uint32_t n_pages) {
    uint32_t i, j;
    uint32_t first_free = 0;
    uint32_t continuous_pages = 0;
    for (i=0; i<kasa_data.pages.length; i++) {
        if (bitset_get_bit(&kasa_data.pages, i) == 0) {
            // This page is free
            if (continuous_pages == 0)
                first_free = i;

            ++continuous_pages;

            if (continuous_pages == n_pages) {
                // set and return
                for (j=first_free; j<first_free+continuous_pages; j++) {
                    bitset_set_bit(&kasa_data.pages, j);
                }
                return (void *)(first_free * kasa_data.page_size);
            }
        } else {
            // This page is taken
            continuous_pages = 0;
        }
    }

    return NULL;
}

k_return_t asa_free(void *addr, uint32_t n_pages) {
    // TODO: do some validation
    uintptr_t addr_int = (uintptr_t)addr;
    uint32_t start = addr_int / kasa_data.page_size;
    uint32_t i;
    for (i=start; i<n_pages; i++) {
        bitset_clear_bit(&kasa_data.pages, i);
    }
    return K_SUCCESS;
}
