#pragma once

#include <stdint.h>

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
#define PF_ID (0x10)           // Was the fault caused by an instruction fetch?

struct page_table_entry {
    uint32_t *addr; // Physical address of the page table
    //uint32_t size; // Number of entries out of 1024 in this table
};
typedef struct page_table_entry page_table_entry_t;

void i386_paging_init();
uint32_t i386_allocate_page(uint32_t address, uint32_t pt_flags, uint32_t pd_flags);
uint8_t i386_free_page(uint32_t address);
uint32_t i386_identity_map_page(uint32_t address, uint32_t pt_flags, uint32_t pd_flags);
void __i386_page_fault_handler(i386_registers_t *r);
