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

extern void load_page_dir(uint32_t *);
extern void enable_paging();
extern uint32_t get_faulting_address();
extern void invlpg(void *);
extern void flush_tlb();

// Kernel mmu data
// Contains kernel's page directory and tables
i386_mmu_data_t i386_kernel_mmu_data;

// A virtual page whose physical address mapping can be changed freely
// Used to access page tables from their physical address
static uint32_t *WINDOW_PAGE = NULL;

// Kernel paging interface implementation
const kpaging_interface_t i386_paging_interface = {
    .kpage_allocate = __i386_kpage_allocate,
    .kpage_free = __i386_kpage_free,
    .kpage_identity_map = __i386_kpage_identity_map,
    .kpage_get_phys = __i386_kpage_get_phys,
};

/**
 * Bool representing whether or not early paging init is done.
 * Should be used to determine which virtual allocator to use.
 * Set to true at end of i386_paging_init.
 *
 * When false, new pages can simply be obtained by the i386 mem placement
 * allocator. Otherwise, the i386 page frame allocator + WINDOW_PAGE should be used
 * to obtain a valid physical page and a way to write to it.
 */
bool early_init_done = false;

/**
 * Initalize an empty i386_paging_data struct.
 * Must be called before the struct is used.
 */
k_return_t i386_mmu_data_init(i386_mmu_data_t *this) {
    uint32_t *tmp;

    // Allocate page directory
    tmp = (uint32_t *)kmalloc_a(sizeof(uint32_t) * 1024, KALLOC_CRITICAL);
    if (!tmp) return K_OOM;
    this->page_directory = (uint32_t)tmp;

    // Mark all page directory entries as not present
    memset32(tmp, PD_RW, 1024);

    return K_SUCCESS;
}


void i386_paging_init() {
    uint32_t i;
    k_return_t ret;

    // Allocate kernel paging data
    ret = i386_mmu_data_init(&i386_kernel_mmu_data);
    if (K_FAILED(ret)) {
        // If we don't have enough memory to allocate the kernel mmu data, panic
        PANIC("Unable to allocate kernel mmu data! Not enough RAM?");
    }

    // Identity map from 0x0000 to the end of KVIRT_RESERVED
    for (i=0; i<KVIRT_RESERVED; i += 0x1000) {
        i386_identity_map_page(&i386_kernel_mmu_data, i, PT_PRESENT | PT_RW, PD_PRESENT | PD_RW);
        if (i > KVIRT_RESERVED) {
            PANIC("Early i386 mem allocations grew past KVIRT_MAX");
        }
    }

    // Install page fault handler
    isr_install_handler(14, __i386_page_fault_handler);

    // Enable paging
    load_page_dir((uint32_t *)i386_kernel_mmu_data.page_directory);
    enable_paging();

    /**
     * Install paging functions into kernel paging interface
     */

    // Install functions
    kpaging_data.interface = &i386_paging_interface;

    // Set kernel start
    kpaging_data.kernel_start = meminfo.kernel_reserved_start;

    // Set kernel end to the end of the kernel binary + the early heap
    kpaging_data.kernel_end = meminfo.kernel_heap_curpos;

    // Set highest kernel-owned page
    kpaging_data.highest_page = meminfo.kernel_heap_start + EARLY_HEAP_MAXSIZE;

    // Make sure there is enough room for the all allocated data + a window page
    ASSERT(kpaging_data.highest_page + PAGE_SIZE <= KVIRT_RESERVED);
    WINDOW_PAGE = (uint32_t *)(kpaging_data.highest_page + PAGE_SIZE);

    // Set page size
    kpaging_data.page_size = PAGE_SIZE;

    // Set total memory
    kpaging_data.mem_total = meminfo.mem_lower + meminfo.mem_upper;

    early_init_done = true;
}

/**
 * Helper function to get a virtual pointer to a given physical address.
 * Generally used to obtain a virtual pointer to a paging structure in memory (hence the return type).
 *
 * !!!WARNING!!! Subsequent calls to this function WILL INVALIDATE PREVIOUS POINTERS.
 * If you obtain a virtual pointer then call another function which obtains its own
 * virtual pointer YOUR ORIGINAL POINTER WILL BE INVALID! To be safe, you should assume
 * that all virtual pointers must be reobtained by calling this function after ANY function call.
 *
 * @param phys_addr  address to obtain virtual pointer to
 * @return           virtual pointer
 */
static uint32_t *get_virt_ptr(uint32_t phys_addr) {
    ASSERT(phys_addr % 0x1000 == 0);
    if (!early_init_done) {
        // If early init isn't done, the physical and virtual memory layout are identical
        return (uint32_t *)phys_addr;
    }

    // Otherwise, move the window page to point
    uint32_t page_index = (uint32_t)WINDOW_PAGE / PAGE_SIZE;
    uint32_t table_index = page_index / 1024;
    uint32_t page_index_in_table = page_index % 1024;

    // Since the WINDOW_PAGE was identity mapped along with its parent, we can directly access it
    ((uint32_t *)TABLE_IN_DIR(table_index, i386_kernel_mmu_data.page_directory))[page_index_in_table]
        = phys_addr | PT_PRESENT | PT_RW;

    // We have to invlpg to prevent a cached window page from being used
    // TODO: how expensive is this? is there a better way to do this?
    invlpg(WINDOW_PAGE);

    return WINDOW_PAGE;
}

/**
 *  Helper function to allocate a page to be used as a page table
 *  @param[out] phys_out  pointer to uint32_t to store physical address of page table at
 *  @return               kernel return code
 */
static k_return_t allocate_page_table(uint32_t *phys_out) {
    uint32_t phys; // Physical address of newly allocated page table
    uint32_t *virt; // Virtual pointer to access newly allocated page table at
    if (early_init_done) {
        // To allocate a page table after early init, we must use the frame allocator and window page
        uint32_t frame = i386_mem_allocate_frame();
        if (!frame) return K_OOM;

        phys = frame * PAGE_SIZE;
        virt = get_virt_ptr(phys);
    } else {
        // To allocate a page during early init, kmalloc can be used
        virt = (uint32_t *)kmalloc_ap(PAGE_SIZE, &phys, KALLOC_CRITICAL);
        if (!virt) return K_OOM;
    }

    // Mark all entries in the page as R/W, not present
    memset32((void *)virt, PT_RW, 1024);

    *phys_out = phys;

    return K_SUCCESS;
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
    uint32_t *pd_virt = get_virt_ptr(this->page_directory);
    if ((pd_virt[table_index] & PD_PRESENT) == 0) {
        return 0;
    }

    // Check if this page is present
    uint32_t *pt_virt = get_virt_ptr(TABLE_IN_DIR(table_index, this->page_directory));
    uint32_t page = pt_virt[page_index_in_table];
    if ((page & PT_PRESENT) == 0) {
        return 0;
    }

    // Remove flags from page and return
    return page & 0xFFFFF000;
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
    if (!frame) {
        return K_OOM;
    }

    // Check if this table is present and allocate it if not
    uint32_t *pd_virt = get_virt_ptr(this->page_directory);
    if ((pd_virt[table_index] & PD_PRESENT) == 0) {
        uint32_t pt_phys;
        ret = allocate_page_table(&pt_phys);
        if (K_FAILED(ret)) {
            i386_mem_free_frame(frame);
            return ret;
        }
        // The call to allocate_page_table invalidated our pd_virt ptr, get it again
        pd_virt = get_virt_ptr(this->page_directory);
        pd_virt[table_index] = pt_phys | pd_flags;
    }

    // Get a virtual pointer to the parent page table
    uint32_t *pt_virt = get_virt_ptr(TABLE_IN_DIR(table_index, pd_virt));

    // Update page in page table
    page = (frame * PAGE_SIZE) | pt_flags;
    pt_virt[page_index_in_table] = page;
    if (out)
        *out = page & 0xFFFFF000;

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

    // Skip request if its table does not exist
    uint32_t *pd_virt = get_virt_ptr(this->page_directory);
    if ((pd_virt[table_index] & PD_PRESENT) == 0) {
        return K_INVALOP;
    }

    // Get frame address and set page to not present, read/write, supervisor
    uint32_t *pt_virt = get_virt_ptr(TABLE_IN_DIR(table_index, pd_virt));
    uint32_t phys_addr_index = pt_virt[page_index_in_table] / PAGE_SIZE;
    pt_virt[page_index_in_table] = PT_RW;

    // Free page frame
    bitset_clear_bit(&i386_mem_frame_bitset, phys_addr_index);

    // Invalidate the address
    invlpg((void *)address);

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
    uint32_t *pd_virt = get_virt_ptr(this->page_directory);
    if ((pd_virt[table_index] & PD_PRESENT) == 0) {
        uint32_t pt_phys;
        ret = allocate_page_table(&pt_phys);
        if (K_FAILED(ret)) return ret;

        // The call to allocate_page_table invalidated our pd_virt ptr, get it again
        pd_virt = get_virt_ptr(this->page_directory);
        pd_virt[table_index] = pt_phys | pd_flags;
    }

    // Get a virtual pointer to the parent page table
    uint32_t *pt_virt = get_virt_ptr((uint32_t)TABLE_IN_DIR(table_index, pd_virt));

    // Update page in page table
    page = (page_index * PAGE_SIZE) | pt_flags;
    pt_virt[page_index_in_table] = page;

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
    printf("EIP: 0x%x\n", r->eip);
    printf("Faulting address: 0x%x\n", faulting_address);
    if (present) printf("Page not present\n");
    if (rw) printf("Page not writable\n");
    if (us) printf("Page not writable from user-mode\n");
    if (reserved) printf("Page reserved bits overwitten\n");
    abort();
}
