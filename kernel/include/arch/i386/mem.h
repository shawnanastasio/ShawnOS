#pragma once

#include <stdint.h>
#include <arch/i386/multiboot.h>

#define PAGE_SIZE 4096

/**
 * Size in bytes of the kernel heap in bytes.
 */
#define KERNEL_HEAP_SIZE 10240

void i386_mem_init(multiboot_info_t *mboot_header);
uint32_t i386_mem_allocate_frame();
uint32_t i386_mem_get_next_frame();
uint32_t i386_mem_peek_frame(uint32_t *counter);
uint32_t i386_mem_get_frame_start_addr(uint32_t num);
uint32_t i386_mem_get_frame_num(uint32_t addr);
uint32_t _i386_mmap_read(uint32_t num, uint8_t mode);
void _i386_elf_sections_read();
uint32_t i386_mem_find_heap(uint32_t size);
bool i386_mem_check_reserved(uint32_t addr);
uintptr_t i386_mem_kmalloc_real(uint32_t size, bool align, uintptr_t *phys);
uintptr_t i386_mem_kmalloc(uint32_t size);
uintptr_t i386_mem_kmalloc_a(uint32_t size);
uintptr_t i386_mem_kmalloc_p(uint32_t size, uintptr_t *phys);
uintptr_t i386_mem_kmalloc_ap(uint32_t size, uintptr_t *phys);
