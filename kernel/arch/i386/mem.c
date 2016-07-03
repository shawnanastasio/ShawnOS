/**
* Architecture-specific memory allocation functions
*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <kernel/kernel.h>
#include <kernel/kernel_thread.h>

#include <arch/i386/multiboot.h>
#include <arch/i386/elf.h>
#include <arch/i386/mem.h>

/**
 * Initalize free memory and pass information to kernel_mem frame allocator
 */
void i386_mem_init(multiboot_info_t *mboot_header) {
    // Ensure multiboot has supplied required information
    // Check for memory map
    if ((mboot_header->flags & (1<<6)) == 0) { // Bit 6 signifies presence of mmap
        goto multiboot_info_fail;
    }

    // Check for ELF section information
    if ((mboot_header->flags & (1<<5)) == 0) { // Bit 5 signifies presence of ELF info
        goto multiboot_info_fail;
    }

    // Parse memory map
    _i386_mmap_read(mboot_header->mmap_length, (multiboot_memory_map_t *) mboot_header->mmap_addr);

    // Parse ELF sections
    _i386_elf_sections_read(&(mboot_header->u.elf_sec));

    return;
multiboot_info_fail:
    printf("Multiboot information structure does not contain required sections!\n");
    abort();
}

/**
 * Read multiboot memory map and determine free memory chunks
 */
void _i386_mmap_read(uint32_t mmap_length, multiboot_memory_map_t *mmap) {
    printk_debug("Parsing multiboot memory map.");

    uintptr_t cur_mmap_addr = (uintptr_t)mmap;
    uintptr_t mmap_end_addr = cur_mmap_addr + mmap_length;
    while (cur_mmap_addr < mmap_end_addr) {
        multiboot_memory_map_t *current_entry = (multiboot_memory_map_t *)cur_mmap_addr;

        // Print out information for this entry
        printf("[mem] addr: 0x%lx len: 0x%lx\n", current_entry->addr, current_entry->len);

        // Increment by the size to get to the next entry
        cur_mmap_addr += current_entry->size + sizeof(uintptr_t);
    }
}

/**
 * Read ELF32 section headers and determine kernel reserved memory
 */
void _i386_elf_sections_read(multiboot_elf_section_header_table_t *elf_sec) {
    // Get first section headers
    elf_section_header_t *cur_header = (elf_section_header_t *)elf_sec->addr;

    // Print initial information about ELF section header availability
    printf("[elf] First ELF section header at 0x%x, num_sections: 0x%x, size: 0x%x\n",
    elf_sec->addr, elf_sec->num, elf_sec->size);

    // Skip to first non-null section header
    ++cur_header;

    // Print all section headers
    uint32_t i;
    for (i=0; i<(elf_sec->num)-1; i++) {
        if (i == 0 || i == (elf_sec->num)-2)
        printf("[elf] name: %d addr: 0x%x size: 0x%x flags: 0x%x offset: 0x%x\n",
        cur_header->sh_name, cur_header->sh_addr, cur_header->sh_size, cur_header->sh_flags, cur_header->sh_offset);

        // Increment to get to the next header
        ++cur_header;
    }
}
