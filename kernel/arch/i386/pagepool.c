#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <kernel/kernel.h>
#include <arch/i386/pagepool.h>

void i386_pagepool_init(i386_pagepool_t *pool) {
    pool->n_free = 0;
    pool->enable_threshold_check = true;
    for (uint32_t i=0; i<PAGEPOOL_ENTRIES; i++) {
        pool->entries[i].valid = false;
        pool->entries[i].phys = 0;
        pool->entries[i].virt = 0;
    }
}

k_return_t i386_pagepool_allocate(i386_pagepool_t *pool, i386_pagepool_entry_t *out) {
    for (uint32_t i=0; i<PAGEPOOL_ENTRIES; i++) {
        if (pool->entries[i].valid) {
            memcpy(out, &pool->entries[i], sizeof(i386_pagepool_entry_t));
            pool->entries[i].valid = false;
            pool->n_free--;
            return K_SUCCESS;
        }
    }

    return K_OOM;
}

k_return_t i386_pagepool_insert(i386_pagepool_t *pool, uintptr_t phys, uintptr_t virt) {
    for (uint32_t i=0; i<PAGEPOOL_ENTRIES; i++) {
        if (!pool->entries[i].valid) {
            // Found an invalid (empty) entry, write the given data to it
            pool->entries[i].phys = phys;
            pool->entries[i].virt = virt;
            pool->entries[i].valid = true;
            pool->n_free++;
            return K_SUCCESS;
        }
    }

    // If there are no free entries return K_INVALOP
    return K_INVALOP;
}
