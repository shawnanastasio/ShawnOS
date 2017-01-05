#pragma once

/**
 * Kernel interface for virtual memory allocation
 * Provides kmalloc()
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct kalloc_interface {
    /**
     * Interface to allocate memory for the kernel
     * @param size size of memory to allocate
     * @param align whether or not to align memory to page bounds
     * @param return physical address of allocated memory to specified pointer
     *        or NULL
     * @return address of allocated memory
     */
    uintptr_t (*kalloc_malloc_real)(size_t size, bool align, uintptr_t *phys);

    /**
     * Interface to free memory previously allocated to the kernel
     * @param addr address of memory allocation to free
     */
    void (*kalloc_free)(uintptr_t addr);
};
typedef struct kalloc_interface kalloc_interface_t;

extern kalloc_interface_t kalloc_data;

uintptr_t *kalloc_malloc_real(size_t size, bool align, uintptr_t *phys);
uintptr_t *kmalloc_a(uintptr_t size);
uintptr_t *kmalloc_p(uintptr_t size, uintptr_t *phys);
uintptr_t *kmalloc_ap(uintptr_t size, uintptr_t *phys);
uintptr_t *kmalloc(size_t size);
void kfree(uintptr_t *addr);
