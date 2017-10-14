/**
 * Kernel interface for architecture-specific paging/MMU operations
 */
#include <stdint.h>
#include <stdbool.h>

#include <kernel/kernel.h>
#include <mm/paging.h>

/**
 * Data structure containing function pointers to currently installed paging
 * system
 */
kpaging_interface_t kpaging_data;

/**
 * Allocate a page using the installed paging system
 * @param  addr  Virtual memory address to allocate page at
 * @param  flags Bitfield containing settings for page
 * @return bool function success
 */
bool kpage_allocate(uintptr_t addr, uint32_t flags) {
    // Make sure that the function is installed
    ASSERT(kpaging_data.kpage_allocate);

    return kpaging_data.kpage_allocate(addr, flags);
}

/**
 * Free a page using the installed paging system
 * @param  addr Virtual memory address of page to free
 * @return bool function success
 */
bool kpage_free(uintptr_t addr) {
    // Make sure that the function is installed
    ASSERT(kpaging_data.kpage_free);

    return kpaging_data.kpage_free(addr);
}

/**
 * Identity-map a page using the installed paging system
 * @param addr Physical memory to map corresponding virtual page to
 * @param flags Bitfield containing settings for page
 * @return bool function success
 */
bool kpage_identity_map(uintptr_t addr, uint32_t flags) {
    // Make sure that the function is installed
    ASSERT(kpaging_data.kpage_identity_map);

    return kpaging_data.kpage_identity_map(addr, flags);
}

uintptr_t kpage_get_phys(uintptr_t addr) {
    // Make sure that the function is installed
    ASSERT(kpaging_data.kpage_get_phys);

    return kpaging_data.kpage_get_phys(addr);
}
