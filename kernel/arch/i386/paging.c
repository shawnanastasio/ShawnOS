/**
 * Paging functions
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <kernel/kernel.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <mm/alloc.h>
#include <mm/asa.h>
#include <kernel/bitset.h>
#include <arch/i386/mem.h>
#include <arch/i386/isr.h>
#include <arch/i386/multiboot.h>
#include <arch/i386/paging.h>
#include <arch/i386/pagepool.h>

extern void load_page_dir(uint32_t *);
extern void enable_paging();
extern uint32_t get_faulting_address();

// Kernel mmu data
// Contains kernel's page directory and tables
i386_mmu_data_t i386_kernel_mmu_data;

// Page pool for storing empty pages to use as page tables
i386_pagepool_t pagepool;

#define TABLE_IN_DIR(table, dir) ( (uint32_t *)((((uint32_t *)(dir))[(uint32_t)(table)]) & 0xFFFFF000) )

/**
 * Initalize an empty i386_paging_data struct.
 * Must be called before the struct is used.
 */
k_return_t i386_mmu_data_init(i386_mmu_data_t *this) {
    uintptr_t tmp;
    uint32_t i;

    // Allocate page directory
    tmp = (uintptr_t)kmalloc_a(sizeof(uint32_t) * 1024, KALLOC_CRITICAL);
    if (!tmp) return K_OOM;
    this->page_directory = (uint32_t *)tmp;

    // Mark all page directory entries as not present
    for (i=0; i<1024; i++) {
        this->page_directory[i] = PD_RW;
    }

    // Allocate virtual page table list
    tmp = (uintptr_t)kmalloc_a(sizeof(uint32_t) * 1024, KALLOC_CRITICAL);
    if (!tmp) {
        kfree(this->page_directory);
        return K_OOM;
    }
    this->page_tables_virt = (uint32_t **)tmp;

    return K_SUCCESS;
}


void i386_paging_init() {
    uint32_t i;
    k_return_t ret;

    // Allocate kernel paging data
    k_return_t res = i386_mmu_data_init(&i386_kernel_mmu_data);
    if (K_FAILED(res)) {
        // If we don't have enough memory to allocate the kernel mmu data, panic
        PANIC("Unable to allocate kernel mmu data! Not enough RAM?");
    }

    // Allocate page pool and fill all entries
    i386_pagepool_init(&pagepool);
    for (i=0; i<PAGEPOOL_ENTRIES; i++) {
        uintptr_t tmp = i386_mem_kmalloc(PAGE_SIZE);
        ASSERT(tmp && "Failed to allocate pages for i386 pagepool!");
        ret = i386_pagepool_insert(&pagepool, tmp, tmp);
        ASSERT(ret == K_SUCCESS && "Failed to insert i386 pagepool entry!");
    }

    // Identity map from 0x0000 to the end of the kernel heap
    for (i=0; i<meminfo.kernel_heap_start + EARLY_HEAP_MAXSIZE; i += 0x1000) {
        i386_identity_map_page(&i386_kernel_mmu_data, i, PT_PRESENT | PT_RW, PD_PRESENT | PD_RW);
        if (i > KVIRT_RESERVED) {
            PANIC("Early i386 mem allocations grew past KVIRT_MAX");
        }
    }

    // Install page fault handler
    isr_install_handler(14, __i386_page_fault_handler);

    // Enable paging
    load_page_dir(i386_kernel_mmu_data.page_directory);
    enable_paging();

    /**
     * Install paging functions into kernel paging interface
     */

    // Install kpage_allocate
    kpaging_data.kpage_allocate = __i386_kpage_allocate;

    // Install kpage_free
    kpaging_data.kpage_free = __i386_kpage_free;

    // Install kpage_identity_map
    kpaging_data.kpage_identity_map = __i386_kpage_identity_map;

    // Install kpage_get_phys
    kpaging_data.kpage_get_phys = __i386_kpage_get_phys;

    // Set kernel start
    kpaging_data.kernel_start = meminfo.kernel_reserved_start;

    // Set kernel end to the end of the kernel binary + the early heap
    kpaging_data.kernel_end = meminfo.kernel_heap_curpos;

    // Set highest kernel-owned page
    kpaging_data.highest_page = meminfo.kernel_heap_start + EARLY_HEAP_MAXSIZE;
    ASSERT(kpaging_data.highest_page <= KVIRT_MAX);

    // Set page size
    kpaging_data.page_size = PAGE_SIZE;

    // Set total memory
    kpaging_data.mem_total = meminfo.mem_lower + meminfo.mem_upper;

    i386_kernel_mmu_data.early_init_done = true;
}


/**
 * Get the physical address for a given virtual addres
 * @param address virtual address to look up in page tables
 * @return found physical address, or 0 if virtual address not in page tables
 */
uint32_t i386_page_get_phys(i386_mmu_data_t *this, uint32_t address) {
    uint32_t page_index = address / PAGE_SIZE;
    uint32_t table_index = page_index / 1024;
    uint32_t page_index_in_table = page_index % 1024;

    // Check if this page table is present
    if ((this->page_directory[table_index] & PD_PRESENT) == 0) {
        return 0;
    }

    // Check if this page is present
    uint32_t page = this->page_tables_virt[table_index][page_index_in_table];
    if ((page & PT_PRESENT) == 0) {
        return 0;
    }

    // Remove flags from page and return
    return page & 0xFFFFF000;
}

/**
 * Allocate contiguous pages and map them.
 * Used to obtain pages to refill the pagepool
 */
k_return_t i386_allocate_empty_pages(i386_mmu_data_t *this, uint32_t n_pages, uintptr_t *phys_out,
                                     uintptr_t *virt_out) {
    if (!this->early_init_done) {
        // Early init not done, we can simply use the placement allocator
        uintptr_t addr = (uintptr_t)kmalloc(n_pages * PAGE_SIZE, KALLOC_CRITICAL);
        if (!addr) return K_OOM;

        // Since we're in paging early init, physical and virtual addresses are the same
        *phys_out = addr;
        *virt_out = addr;
        return K_SUCCESS;
    } else {
        // Early init is done, we'll need to grab the pages from ASA and then map them
        // with i386_allocate_page
        uintptr_t virt_addr;
        uintptr_t phys_addr;
        uint32_t i;
        k_return_t ret;

        // Grab virtual pages from ASA
        virt_addr = (uintptr_t)asa_alloc(n_pages);
        if (!virt_addr) return K_OOM;

        // Map first the virtual page to physical memory and save the starting pointer
        ret = i386_allocate_page(this, virt_addr, PT_PRESENT | PT_RW, PD_PRESENT | PD_RW, &phys_addr);
        if (K_FAILED(ret)) {
            asa_free((void * )virt_addr, n_pages);
            return ret;
        }

        // Map the rest
        for (i=1; i<n_pages; i++) {
            ret = i386_allocate_page(this, virt_addr + (i * PAGE_SIZE), PT_PRESENT | PT_RW,
                                     PD_PRESENT | PD_RW, NULL);
            if (K_FAILED(ret)) {
                asa_free((void *)virt_addr, n_pages);
                while (i-- > 0) {
                    i386_free_page(this, virt_addr + (i * PAGE_SIZE));
                }
                return ret;
            }
        }

        *phys_out = phys_addr;
        *virt_out = virt_addr;
        return K_SUCCESS;
    }
}

/**
 * Allocate a page at the specified address. Will overwrite any current page.
 * @param address virtual address that corresponds to page to allocate
 * @param pt_flags page table entry flags to be used
 * @param pd_flags page directory entry flags to be used if the page directory entry does not exist
 * @param[out] out ptr to place the physical address that the page was mapped to, or NULL to discard
 * @return kernel return code
 */
k_return_t i386_allocate_page(i386_mmu_data_t *this, uint32_t address, uint32_t pt_flags, uint32_t pd_flags,
                              uint32_t *out) {
    ASSERT(address % PAGE_SIZE == 0);
    uint32_t page_index = address / PAGE_SIZE;
    uint32_t table_index = page_index / 1024;
    uint32_t page_index_in_table = page_index % 1024;
    uint32_t page;
    k_return_t ret;

    // Allocate a page frame
    uint32_t frame = i386_mem_allocate_frame();
    if (!frame) return K_OOM;
    //printk_debug("Page at 0x%x will be mapped to phys 0x%x", address, frame*0x1000);

    // Check if this table is present and allocate it from the pagepool if not
    if ((this->page_directory[table_index] & PD_PRESENT) == 0) {
        // Check if the pagepool needs to be refilled
        if (pagepool.n_free <= PAGEPOOL_THRESHOLD && pagepool.enable_threshold_check) {
            // Temporarily disable threshold check while we handle it
            pagepool.enable_threshold_check = false;

            // Allocate PAGEPOOL_ENTRIES - PAGEPOOL_THRESHOLD new pages
            uintptr_t new_phys, new_virt;
            uint32_t n_pages = PAGEPOOL_ENTRIES - PAGEPOOL_THRESHOLD;
            ret = i386_allocate_empty_pages(this, 1, &new_phys, &new_virt);
            if (K_FAILED(ret)) return ret;
            printk_debug("alloc empty pages got new phys: 0x%x virt: 0x%x", new_phys, new_virt); //sponge

            // Insert new pages into pagepool
            uint32_t i;
            for(i=0;i<n_pages;i++) {
                ret = i386_pagepool_insert(&pagepool, new_phys + (i * PAGE_SIZE), new_virt + (i * PAGE_SIZE));
                if (K_FAILED(ret)) {
                    PANIC("Failed to insert some new pages into pagepool!");
                    break;
                }
            }

            // Re-enable the threshold check
            pagepool.enable_threshold_check = true;
        }

        i386_pagepool_entry_t entry;
        ret = i386_pagepool_allocate(&pagepool, &entry);
        if (K_FAILED(ret)) return ret;

        // Wipe all entries (set to R/W, not present)
        uint32_t i;
        for (i=0; i<1024; i++) *((uint32_t *)entry.virt + i) = PT_RW;
        //memset32((void *)entry.virt, PT_RW, 1024);
        printk_debug("Pagepool returned phys: 0x%x virt: 0x%x", entry.phys, entry.virt); //sponge

        this->page_directory[table_index] = entry.phys | pd_flags;
        this->page_tables_virt[table_index] = (uint32_t *)entry.virt;
    }

    // Update page in page table
    page = (frame * PAGE_SIZE) | pt_flags;
    this->page_tables_virt[table_index][page_index_in_table] = page;
    //printk_debug("provind phys out: 0x%x", (frame * PAGE_SIZE)); //sponge
    if (out) {
        *out = page & 0xFFFFF000;
    }

    return K_SUCCESS;
}

/**
 * Free the page at the specified address and it's corresponding frame.
 * @param  address virtual address that corresponds to page to free
 * @return K_SUCCESS or K_INVALOP if page is already free
 */
k_return_t i386_free_page(i386_mmu_data_t *this, uint32_t address) {
    ASSERT(address % PAGE_SIZE == 0);
    uint32_t page_index = address / PAGE_SIZE;
    uint32_t table_index = page_index / 1024;
    uint32_t page_index_in_table = page_index % 1024;
    uint32_t page = this->page_tables_virt[table_index][page_index_in_table];

    // Skip request if it's table does not have a Page Directory Entry
    if ((this->page_directory[table_index] & PD_PRESENT) == 0) {
        return K_INVALOP;
    }

    // Get page's physical frame address
    uint32_t phys_addr_index = page / PAGE_SIZE;

    // Set page to not present, read/write, supervisor
    this->page_tables_virt[table_index][page_index_in_table] = PT_RW;

    // Free page frame
    bitset_clear_bit(&i386_mem_frame_bitset, phys_addr_index);

    return K_SUCCESS;
}

/**
 * Identity map a page so that it corresponds with physical memory
 * @param address virtual address that corresponds to page to allocate
 * @param pt_flags page table entry flags to be used
 * @param pd_flags page directory entry flags to be used if the page directory entry does not exist
 * @return the created page table entry
 */
uint32_t i386_identity_map_page(i386_mmu_data_t *this, uint32_t address, uint32_t pt_flags, uint32_t pd_flags) {
    ASSERT(address % PAGE_SIZE == 0);
    uint32_t page_index = address / PAGE_SIZE;
    uint32_t table_index = page_index / 1024;
    uint32_t page_index_in_table = page_index % 1024;
    uint32_t page;
    k_return_t ret;

    // Check if this table is present and allocate it if not
    if ((this->page_directory[table_index] & PD_PRESENT) == 0) {
        // Check if the pagepool needs to be refilled
        if (pagepool.n_free <= PAGEPOOL_THRESHOLD && pagepool.enable_threshold_check) {
            // Temporarily disable threshold check while we handle it
            pagepool.enable_threshold_check = false;

            // Allocate PAGEPOOL_ENTRIES - PAGEPOOL_THRESHOLD new pages
            uintptr_t new_phys, new_virt;
            uint32_t n_pages = PAGEPOOL_ENTRIES - PAGEPOOL_THRESHOLD;
            ret = i386_allocate_empty_pages(this, n_pages, &new_phys, &new_virt);
            if (K_FAILED(ret)) return ret;

            // Insert new pages into pagepool
            uint32_t i;
            for(i=0;i<n_pages;i++) {
                ret = i386_pagepool_insert(&pagepool, new_phys + (i * PAGE_SIZE), new_virt + (i * PAGE_SIZE));
                if (K_FAILED(ret)) {
                    printk_debug("Only allocated %d out of %d pages.", i+1, n_pages);
                    PANIC("Failed to insert some new pages into pagepool!");
                    break;
                }
            }

            // Re-enable the threshold check
            pagepool.enable_threshold_check = true;
        }

        i386_pagepool_entry_t entry;
        ret = i386_pagepool_allocate(&pagepool, &entry);
        if (K_FAILED(ret)) return ret;

        this->page_directory[table_index] = entry.phys | pd_flags;
        this->page_tables_virt[table_index] = (uint32_t *)entry.virt;
    }

    // Update page in page table
    page = (page_index * PAGE_SIZE) | pt_flags;
    this->page_tables_virt[table_index][page_index_in_table] = page;

    // Mark this page frame as allocated in the bitset
    bitset_set_bit(&i386_mem_frame_bitset, page_index);

    return page;
}

/**
 * Kernel paging interface functions. Should not be called directly
 */
k_return_t __i386_kpage_allocate(uintptr_t addr, uint32_t flags) {
    return i386_allocate_page(&i386_kernel_mmu_data, addr, flags, flags, NULL);
}

k_return_t __i386_kpage_free(uintptr_t addr) {
    return i386_free_page(&i386_kernel_mmu_data, addr);
}

k_return_t __i386_kpage_identity_map(uintptr_t addr, uint32_t flags) {
    uint32_t tmp = i386_identity_map_page(&i386_kernel_mmu_data, addr, flags, flags);
    if (tmp) return K_SUCCESS;
    return K_OOM;
}

uintptr_t __i386_kpage_get_phys(uintptr_t addr) {
    return i386_page_get_phys(&i386_kernel_mmu_data, addr);
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
