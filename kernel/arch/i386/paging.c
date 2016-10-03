/**
 * Paging functions
 */

#include <stdint.h>
#include <stdio.h>

#include <kernel/kernel_mem.h>
#include <kernel/bitset.h>
#include <arch/i386/mem.h>

extern void load_page_dir(uint32_t *);
extern void enable_paging();

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

    // Identity map from 0x0000 to the end of the kernel binary
    for (i=0; i<meminfo.kernel_reserved_end; i += 0x1000) {
        i386_identity_map_page(i, PT_PRESENT | PT_RW, PD_PRESENT | PD_RW);
    }

    // Enable paging
    load_page_dir(i386_page_directory);
    enable_paging();
}

/**
 * Allocate a page at the specified address. Will overwrite any current page.
 * @param address virtual address that corresponds to page to allocate
 * @param pt_flags page table entry flags to be used
 * @param pd_flags page directory entry flags to be used if the page directory does not exist
 * @return the created page table entry
 */
uint32_t i386_allocate_page(uint32_t address, uint32_t pt_flags, uint32_t pd_flags) {
    uint32_t page_index = address / 0x1000;
    uint32_t table_index = page_index / 1024;
    uint32_t page_index_in_table = page_index % 1024;
    uint32_t page = page_table_list[table_index].addr[page_index_in_table];

    uint32_t frame = i386_mem_allocate_frame();
    printf("Page at 0x%x will be mapped to phys 0x%x\n", address, frame*0x1000);

    // Update page table entry in page table list
    page_table_list[table_index].addr[page_index_in_table] = (frame * 0x1000) | pt_flags;

    // Check if the page directory contains this table, and add it if not
    if ((i386_page_directory[table_index] & PD_PRESENT) == 0) {
        printf("Adding PD entry at table_index: %u\n", table_index);
        i386_page_directory[table_index] = ((uint32_t)page_table_list[table_index].addr) | pd_flags;
    }

    return page;
}

/**
 * Identity map a page so that it corresponds with physical memory
 * @param address virtual address that corresponds to page to allocate
 * @param pt_flags page table entry flags to be used
 * @param pd_flags page directory entry flags to be used if the page directory does not exist
 * @return the created page table entry
 */
uint32_t i386_identity_map_page(uint32_t address, uint32_t pt_flags, uint32_t pd_flags) {
    uint32_t page_index = address / 0x1000;
    uint32_t table_index = page_index / 1024;
    uint32_t page_index_in_table = page_index % 1024;
    uint32_t page = page_table_list[table_index].addr[page_index_in_table];

    // Update page table entry in page table list
    page_table_list[table_index].addr[page_index_in_table] = page_index * 0x1000 | pt_flags;

    // Mark this page frame as allocated in the bitset
    bitset_set_bit(i386_mem_frame_bitset, page_index);

    // Check if the page directory contains this table, and add it if not
    if ((i386_page_directory[table_index] & PD_PRESENT) == 0) {
        printf("Adding PD entry at table_index: %u\n", table_index);
        i386_page_directory[table_index] = ((uint32_t)page_table_list[table_index].addr) | pd_flags;
    }

    return page;
}
