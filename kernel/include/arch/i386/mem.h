#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <kernel/bitset.h>

#include <arch/i386/multiboot.h>
#include <arch/i386/paging.h>

#define PAGE_SIZE 4096

#define EARLY_HEAP_MAXSIZE 0x50000

/**
 * Enum declaring memory state values
 */
enum mem_states_t {
    MEM_RESERVED,
    MEM_FREE,
    MEM_NONEXISTANT
};

/**
 * Bitset containing the status of all frame numbers
 */
extern bitset_t i386_mem_frame_bitset;

/**
 * Structure containing internal data for i386 memory functions
 */
struct i386_mem_info {
    multiboot_info_t *mboot_header;
    uint32_t mmap_length;
    multiboot_memory_map_t *mmap;
    multiboot_elf_section_header_table_t *elf_sec;
    uint32_t kernel_reserved_start;
    uint32_t kernel_reserved_end;
    uint32_t multiboot_reserved_start;
    uint32_t multiboot_reserved_end;
    uint32_t kernel_heap_start;
    uint32_t kernel_heap_curpos;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t highest_free_address;
};
typedef struct i386_mem_info i386_mem_info_t;
extern i386_mem_info_t meminfo;

void i386_mem_init(multiboot_info_t *mboot_header);
void _i386_mem_init_bitset();
uint32_t i386_mem_allocate_frame();
void i386_mem_free_frame(uint32_t frame);
uint32_t i386_mem_get_next_free_frame();
uint32_t i386_mem_get_frame_start_addr(uint32_t num);
uint32_t i386_mem_get_frame_num(uint32_t addr);
void _i386_elf_sections_read();
uint8_t i386_mem_check_reserved(uint32_t addr);
uintptr_t i386_mem_kmalloc_real(uint32_t size, bool align, uintptr_t *phys);
uintptr_t i386_mem_kmalloc(uint32_t size);
uintptr_t i386_mem_kmalloc_a(uint32_t size);
uintptr_t i386_mem_kmalloc_p(uint32_t size, uintptr_t *phys);
uintptr_t i386_mem_kmalloc_ap(uint32_t size, uintptr_t *phys);
uint8_t _i386_mmap_check_reserved(uint32_t addr);
void _i386_print_reserved();
