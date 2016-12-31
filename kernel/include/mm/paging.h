#pragma once

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
     * Interface to initalize paging on system boot
     * Called only once
     */
    void (*kpage_init)(void);

    /**
     * Interface to allocate a page
     * @param addr Virtual memory address to allocate page at
     * @param flags Bitfield containing settings for page
     * @return bool function success
     */
    bool (*kpage_allocate)(uintptr_t addr, uint32_t flags);

    /**
     * Interface to free a page
     * @param addr Virtual memory address of page to free
     * @return bool function success
     */
    bool (*kpage_free)(uintptr_t addr);

    /**
     * Interface to identity-map a page
     * @param addr Physical memory to map corresponding virtual page to
     * @param flags Bitfield containing settings for page
     * @return bool function success
     */
    bool (*kpage_identity_map)(uintptr_t addr, uint32_t flags);
};
typedef struct kpaging_interface kpaging_interface_t;

extern kpaging_interface_t kpaging_data;

void kpage_init();
bool kpage_allocate(uintptr_t addr, uint32_t flags);
bool kpage_free(uintptr_t addr);
bool kpage_identity_map(uintptr_t addr, uint32_t flags);
