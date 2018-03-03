#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <arch/i386/isr.h>

// Page Table Entry flags
#define PT_PRESENT (1<<0)      // Is the page present?
#define PT_RW (1<<1)           // Is the page read/write?
#define PT_USER (1<<2)         // Is the page user-mode?
#define PT_WRITETHROUGH (1<<3) // Is writethrough enabled for the page?

// Page Directory Entry flags
#define PD_PRESENT (1<<0)
#define PD_RW (1<<1)
#define PD_USER (1<<2)
#define PD_WRITETHROUGH (1<<3)

// Page Fault error flags
#define PF_PRESENT (1<<0)      // Was the page present?
#define PF_RW (1<<1)           // Was the page wrongfully written to?
#define PF_USER (1<<2)         // Was the CPU in user-mode?
#define PF_RESERVED (1<<3)     // Were the CPU-reserved bytes overwritten?
#define PF_ID (1<<4)           // Was the fault caused by an instruction fetch?

/**
 * Struct containing per-process mmu/paging data.
 * Includes page directory, page tables, etc.
 */
struct i386_mmu_data {
    /**
     * i386 page directory. Contains 1024 page tables.
     */
    uint32_t *page_directory;

    /**
     * Virtual addresses of page tables. Indices correspond 1-1 with page directory.
     * Entries contain physical addresses of page tables
     *
     * Ex. To access the first page in the second page table, do the following:
     * page_tables_virt[2][1]
     */
    uint32_t **page_tables_virt;

    /**
     * Bool representing whether or not early paging init is done.
     * Should be used to determine which virtual allocator to use.
     * Set to true at end of i386_paging_init.
     *
     * When false, new virtual pages can simply be obtained by the i386 mem placement
     * allocator. Otherwise, the kernel ASA + i386_allocate_page should be used
     * to obtain a valid virtual page with a physical mapping.
     */
    bool early_init_done;
};
typedef struct i386_mmu_data i386_mmu_data_t;

extern i386_mmu_data_t i386_kernel_mmu_data;

void i386_paging_init();
uint32_t i386_page_get_phys(i386_mmu_data_t *this, uint32_t address);
k_return_t i386_allocate_empty_pages(i386_mmu_data_t *this, uint32_t n_pages, uintptr_t *phys_out,
                                     uintptr_t *virt_out);
k_return_t i386_allocate_page(i386_mmu_data_t *this, uint32_t address, uint32_t pt_flags, uint32_t pd_flags,
                              uint32_t *out);
k_return_t i386_free_page(i386_mmu_data_t *this, uint32_t address);
uint32_t i386_identity_map_page(i386_mmu_data_t *this, uint32_t address, uint32_t pt_flags, uint32_t pd_flags);
void __i386_page_fault_handler(i386_registers_t *r);

// Kernel paging interface implementation
k_return_t __i386_kpage_allocate(uintptr_t addr, uint32_t flags);
k_return_t __i386_kpage_free(uintptr_t addr);
k_return_t __i386_kpage_identity_map(uintptr_t addr, uint32_t flags);
uintptr_t __i386_kpage_get_phys(uintptr_t addr);