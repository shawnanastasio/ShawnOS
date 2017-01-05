/**
 * Kernel interface for virtual memory allocation
 * Provides kmalloc()
 */

#include <stdint.h>
#include <stddef.h>

#include <kernel/kernel.h>
#include <mm/alloc.h>

/**
 * Data structure containing function pointers to currently installed alloc
 * system
 */
kalloc_interface_t kalloc_data;

uintptr_t *kalloc_malloc_real(size_t size, bool align, uintptr_t *phys) {
    // Make sure that the function is installed
    ASSERT(kalloc_data.kalloc_malloc_real);

    return (uintptr_t *)kalloc_data.kalloc_malloc_real(size, align, phys);
}

uintptr_t *kmalloc_a(uintptr_t size) {
    // Make sure that the function is installed
    ASSERT(kalloc_data.kalloc_malloc_real);

    return (uintptr_t *)kalloc_data.kalloc_malloc_real(size, true, NULL);
}

uintptr_t *kmalloc_p(uintptr_t size, uintptr_t *phys) {
    // Make sure that the function is installed
    ASSERT(kalloc_data.kalloc_malloc_real);

    return (uintptr_t *)kalloc_data.kalloc_malloc_real(size, false, phys);
}

uintptr_t *kmalloc_ap(uintptr_t size, uintptr_t *phys) {
    // Make sure that the function is installed
    ASSERT(kalloc_data.kalloc_malloc_real);

    return (uintptr_t *)kalloc_data.kalloc_malloc_real(size, true, phys);
}

uintptr_t *kmalloc(size_t size) {
    // Make sure that the function is installed
    ASSERT(kalloc_data.kalloc_malloc_real);

    return (uintptr_t *)kalloc_data.kalloc_malloc_real(size, false, NULL);
}

void kfree(uintptr_t *addr) {
    // Make sure that the function is installed
    ASSERT(kalloc_data.kalloc_free);

    kalloc_data.kalloc_free((uintptr_t)addr);
}
