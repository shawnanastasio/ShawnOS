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

void *kalloc_malloc_real(size_t size, uintptr_t *phys, uint32_t flags) {
    // Make sure that the function is installed
    ASSERT(kalloc_data.kalloc_malloc_real);

    return (void *)kalloc_data.kalloc_malloc_real(size, phys, flags);
}

void *kmalloc_a(uintptr_t size, uint32_t flags) {
    // Make sure that the function is installed
    ASSERT(kalloc_data.kalloc_malloc_real);

    return (void *)kalloc_data.kalloc_malloc_real(size, NULL, flags | KALLOC_PAGE_ALIGN);
}

void *kmalloc_p(uintptr_t size, uintptr_t *phys, uint32_t flags) {
    // Make sure that the function is installed
    ASSERT(kalloc_data.kalloc_malloc_real);

    return (void *)kalloc_data.kalloc_malloc_real(size, phys, flags);
}

void *kmalloc_ap(uintptr_t size, uintptr_t *phys, uint32_t flags) {
    // Make sure that the function is installed
    ASSERT(kalloc_data.kalloc_malloc_real);

    return (void *)kalloc_data.kalloc_malloc_real(size, phys, flags | KALLOC_PAGE_ALIGN);
}

void *kmalloc(size_t size, uint32_t flags) {
    // Make sure that the function is installed
    ASSERT(kalloc_data.kalloc_malloc_real);

    return (void *)kalloc_data.kalloc_malloc_real(size, NULL, flags);
}

void kfree(uintptr_t *addr) {
    // Make sure that the function is installed
    ASSERT(kalloc_data.kalloc_free);

    kalloc_data.kalloc_free((uintptr_t)addr);
}
