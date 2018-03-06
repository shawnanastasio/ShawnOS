#pragma once

/**
 * Kernel interface for virtual memory allocation
 * Provides kmalloc()
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


/* Allocation flags */

// Heap priority levels
#define KALLOC_CRITICAL      (1<<0)
#define KALLOC_GENERAL       (1<<1)
 
#define KALLOC_PAGE_ALIGN    (1<<2)

struct kalloc_interface {
    /**
     * Interface to allocate memory for the kernel
     * @param size size of memory to allocate
     * @param return physical address of allocated memory to specified pointer
     *        or NULL
     * @param flags allocation flags (see above)
     * @return address of allocated memory
     */
    uintptr_t (*kalloc_malloc_real)(size_t size, uintptr_t *phys, uint32_t flags);

    /**
     * Interface to free memory previously allocated to the kernel
     * @param addr address of memory allocation to free
     */
    void (*kalloc_free)(uintptr_t addr);
};
typedef struct kalloc_interface kalloc_interface_t;

extern kalloc_interface_t kalloc_data;

void *kalloc_malloc_real(size_t size, uintptr_t *phys, uint32_t flags);
void *kmalloc_a(uintptr_t size, uint32_t flags);
void *kmalloc_p(uintptr_t size, uintptr_t *phys, uint32_t flags);
void *kmalloc_ap(uintptr_t size, uintptr_t *phys, uint32_t flags);
void *kmalloc(size_t size, uint32_t flags);
void kfree(uintptr_t *addr);