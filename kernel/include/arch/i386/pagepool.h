#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <kernel/kernel.h>

#define PAGEPOOL_ENTRIES 20

// Once n_free <= to this value, the pagepool should be refilled
#define PAGEPOOL_THRESHOLD 3

typedef struct i386_pagepool_entry {
    bool valid;
    uintptr_t phys;
    uintptr_t virt;
} i386_pagepool_entry_t;

typedef struct i386_pagepool {
    i386_pagepool_entry_t entries[PAGEPOOL_ENTRIES];
    uint32_t n_free;
    bool enable_threshold_check;
} i386_pagepool_t;

void i386_pagepool_init(i386_pagepool_t *pool);
k_return_t i386_pagepool_allocate(i386_pagepool_t *pool, i386_pagepool_entry_t *out);
k_return_t i386_pagepool_insert(i386_pagepool_t *pool, uintptr_t phys, uintptr_t virt);