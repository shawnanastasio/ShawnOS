#pragma once

#include <stdint.h>
#include <stddef.h>

#include <kernel/kernel.h>

/**
 * Kernel interface for architecture-specific paging/MMU operations
 */

// Page flags
#define KPAGE_PRESENT (1<<0)      // Is the page present?
#define KPAGE_RW (1<<1)           // Is the page read/write?
#define KPAGE_USER (1<<2)         // Is the page user-mode?
#define KPAGE_WRITETHROUGH (1<<3) // Is writethrough enabled for the page?

struct kpaging_interface {
    /**
     * Interface to allocate a page
     * @param addr Virtual memory address to allocate page at
     * @param flags Bitfield containing settings for page
     * @return bool function success
     */
    k_return_t (*kpage_allocate)(uintptr_t addr, uint32_t flags);

    /**
     * Interface to free a page
     * @param addr Virtual memory address of page to free
     * @return bool function success
     */
    k_return_t (*kpage_free)(uintptr_t addr);

    /**
     * Interface to identity-map a page
     * @param addr Physical memory to map corresponding virtual page to
     * @param flags Bitfield containing settings for page
     * @return bool function success
     */
    k_return_t (*kpage_identity_map)(uintptr_t addr, uint32_t flags);

    /**
     * Interface to get the physical address for the given virt address
     * @param addr virtual address to look up
     * @return physical address corresponding to virutal address, or 0 if none
     */
    uintptr_t (*kpage_get_phys)(uintptr_t addr);
};
typedef struct kpaging_interface kpaging_interface_t;

struct kpaging_data {
    // Currently installed paging functions to use
    const kpaging_interface_t *interface;

    // Virtual memory address that the kernel starts at
    uintptr_t kernel_start;

    // Virtual memory address that the kernel ends at
    uintptr_t kernel_end;

    // Virtual memory address of the first contiguous page not owned by the kernel
    uintptr_t highest_page;

    // Size of pages
    uint32_t page_size;

    // Total amount of system memory in kilobytes
    size_t mem_total;
};
typedef struct kpaging_data kpaging_data_t;

extern kpaging_data_t kpaging_data;

void kpage_init();
k_return_t kpage_allocate(uintptr_t addr, uint32_t flags);
k_return_t kpage_free(uintptr_t addr);
k_return_t kpage_identity_map(uintptr_t addr, uint32_t flags);
uintptr_t kpage_get_phys(uintptr_t addr);
