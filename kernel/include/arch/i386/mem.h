#include <stdint.h>
#include <arch/i386/multiboot.h>

void i386_mem_init(multiboot_info_t *mboot_header);
void _i386_mmap_read(uint32_t mmap_length, multiboot_memory_map_t *mmap);
void _i386_elf_sections_read(multiboot_elf_section_header_table_t *elf_sec);
