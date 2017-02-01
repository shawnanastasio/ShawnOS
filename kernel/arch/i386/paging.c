/**
 * Paging functions
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <kernel/kernel.h>
#include <mm/paging.h>
#include <mm/alloc.h>
#include <kernel/bitset.h>
#include <arch/i386/mem.h>
#include <arch/i386/isr.h>
#include <arch/i386/multiboot.h>
#include <arch/i386/paging.h>

extern void load_page_dir(uint32_t *);
extern void enable_paging();
extern uint32_t get_faulting_address();

// Page Directory containing an array of pointers to page tables
// Passed directly to load_page_dir
uint32_t *i386_page_directory;
uint32_t i386_page_directory_size = 0;

// An array of the physical memory addresses and sizes of all currently created page tables.
// For internal use, does not get passed to cpu.
page_table_entry_t page_table_list[1024];


void i386_paging_init() {
    // Allocate the page directory
    i386_page_directory = (uint32_t *)kmalloc_a(sizeof(uint32_t) * 1024);

    // Wipe page directory
    uint32_t i;
    for (i=0; i < 1024; i++) {
        // Set all page directory entries to not present, read/write, supervisor
        i386_page_directory[i] = PD_RW;
    }

    // Identity map from 0x0000 to the end of the kernel heap
    // The kernel heap will grow as we map pages,
    for (i=0; i<=meminfo.kernel_heap_curpos; i += 0x1000) {
        i386_identity_map_page(i, PT_PRESENT | PT_RW, PD_PRESENT | PD_RW);
    }

    // Mark the kernel heap as reserved in the bitset
    for (i=meminfo.kernel_heap_start; i < meminfo.kernel_heap_curpos; i += 0x1000) {
        bitset_set_bit(&i386_mem_frame_bitset, i/0x1000);
    }

    // Install page fault handler
    isr_install_handler(14, __i386_page_fault_handler);

    // Enable paging
    load_page_dir(i386_page_directory);
    enable_paging();

    /**
     * Install paging functions into kernel paging interface
     */

    // Install kpage_allocate
    kpaging_data.kpage_allocate = __i386_kpage_allocate;

    // Install kpage_free
    kpaging_data.kpage_free = i386_free_page;

    // Install kpage_identity_map
    kpaging_data.kpage_identity_map = __i386_kpage_identity_map;

    // Set kernel start
    kpaging_data.kernel_start = meminfo.kernel_reserved_start;

    // Set kernel end to the end of the kernel binary + the early heap
    kpaging_data.kernel_end = meminfo.kernel_reserved_end + EARLY_HEAP_MAXSIZE;

    // Set highest kernel-owned page
    kpaging_data.highest_page = ((kpaging_data.kernel_end + PAGE_SIZE) & 0xFFFFF000) + PAGE_SIZE;

    // Set page size
    kpaging_data.page_size = PAGE_SIZE;
}

/**
 * Allocate a page at the specified address. Will overwrite any current page.
 * @param address virtual address that corresponds to page to allocate
 * @param pt_flags page table entry flags to be used
 * @param pd_flags page directory entry flags to be used if the page directory entry does not exist
 * @return the created page table entry
 */
uint32_t i386_allocate_page(uint32_t address, uint32_t pt_flags, uint32_t pd_flags) {
    ASSERT(address % PAGE_SIZE == 0);
    uint32_t page_index = address / 0x1000;
    uint32_t table_index = page_index / 1024;
    uint32_t page_index_in_table = page_index % 1024;
    uint32_t page = page_table_list[table_index].addr[page_index_in_table];

    uint32_t frame = i386_mem_allocate_frame();
    //printk_debug("Page at 0x%x will be mapped to phys 0x%x", address, frame*0x1000);

    // Update page table entry in page table list
    page_table_list[table_index].addr[page_index_in_table] = (frame * 0x1000) | pt_flags;

    // Check if the page directory contains this table, and add it if not
    if ((i386_page_directory[table_index] & PD_PRESENT) == 0) {
        //printf("Adding PD entry at table_index: %u\n", table_index);
        i386_page_directory[table_index] = ((uint32_t)page_table_list[table_index].addr) | pd_flags;
    }

    return page;
}

/**
 * Free the page at the specified address and it's corresponding frame.
 * @param  address virtual address that corresponds to page to free
 * @return 0 if operation failed, 1 if operation succeeded
 */
bool i386_free_page(uint32_t address) {
    ASSERT(address % PAGE_SIZE == 0);
    uint32_t page_index = address / 0x1000;
    uint32_t table_index = page_index / 1024;
    uint32_t page_index_in_table = page_index % 1024;
    uint32_t page = page_table_list[table_index].addr[page_index_in_table];

    // Skip request if it's table does not have a Page Directory Entry
    if ((i386_page_directory[table_index] & PD_PRESENT) == 0) {
        return 0;
    }

    // Get page's physical frame address
    uint32_t phys_addr_index = page / 0x1000;

    // Set page to not present, read/write, supervisor
    page_table_list[table_index].addr[page_index_in_table] = PT_RW;

    // Free page frame
    bitset_clear_bit(&i386_mem_frame_bitset, phys_addr_index);

    return 1;
}

/**
 * Identity map a page so that it corresponds with physical memory
 * @param address virtual address that corresponds to page to allocate
 * @param pt_flags page table entry flags to be used
 * @param pd_flags page directory entry flags to be used if the page directory entry does not exist
 * @return the created page table entry
 */
uint32_t i386_identity_map_page(uint32_t address, uint32_t pt_flags, uint32_t pd_flags) {
    ASSERT(address % PAGE_SIZE == 0);
    uint32_t page_index = address / 0x1000;
    uint32_t table_index = page_index / 1024;
    uint32_t page_index_in_table = page_index % 1024;
    uint32_t page = page_table_list[table_index].addr[page_index_in_table];

    // Update page table entry in page table list
    page_table_list[table_index].addr[page_index_in_table] = page_index * 0x1000 | pt_flags;

    // Mark this page frame as allocated in the bitset
    bitset_set_bit(&i386_mem_frame_bitset, page_index);

    // Check if the page directory contains this table, and add it if not
    if ((i386_page_directory[table_index] & PD_PRESENT) == 0) {
        //printf("Adding PD entry at table_index: %u\n", table_index);
        i386_page_directory[table_index] = ((uint32_t)page_table_list[table_index].addr) | pd_flags;
    }

    return page;
}

/**
 * Kernel paging interface function for allocating page
 * Redirects to i386_allocate_page
 */
inline bool __i386_kpage_allocate(uintptr_t addr, uint32_t flags) {
    i386_allocate_page(addr, flags, flags);
    // TODO: add actual error handling when paging fails
    return true;
}

/**
 * Kernel paging interface implementation for identity-mapping page
 * Redirects to i386_identity_map_page
 */
inline bool __i386_kpage_identity_map(uintptr_t addr, uint32_t flags) {
    uint32_t tmp = i386_identity_map_page(addr, flags, flags);
    if (tmp) return true;
    return false;
}

void __i386_page_fault_handler(i386_registers_t *r) {
    // Get faulting address
    uint32_t faulting_address = get_faulting_address();

    // Read error flags
    bool present = !(r->err_code & PF_PRESENT);
    bool rw = (r->err_code & PF_RW);
    bool us = (r->err_code & PF_USER);
    bool reserved = (r->err_code & PF_RESERVED);

    // Dump information about fault to screen
    printf("HALT - PAGE FAULT\n\n");
    printf("Faulting address: 0x%x\n", faulting_address);
    if (present) printf("Page not present\n");
    if (rw) printf("Page not writable\n");
    if (us) printf("Page not writable from user-mode\n");
    if (reserved) printf("Page reserved bits overwitten\n");
    abort();
}
