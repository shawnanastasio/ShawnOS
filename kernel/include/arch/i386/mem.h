#pragma once

#include <stdint.h>
#include <arch/i386/multiboot.h>

void i386_mem_init(multiboot_info_t *mboot_header);
uint32_t i386_mem_allocate_frame();
uint32_t i386_mem_get_next_frame();
uint32_t i386_mem_peek_frame(uint32_t *counter);
uint32_t i386_mem_get_frame_start_addr(uint32_t num);
uint32_t i386_mem_get_frame_num(uint32_t addr);
uint32_t _i386_mmap_read(uint32_t num, uint8_t mode);
void _i386_elf_sections_read();
uint32_t i386_mem_kmalloc(uint32_t size);
