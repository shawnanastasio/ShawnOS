/**
 * Architecture-specific memory allocation functions
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <kernel/kernel.h>
#include <kernel/kernel_thread.h>
#include <kernel/mem/kernel_mem.h>

#include <arch/i386/multiboot.h>
#include <arch/i386/elf.h>
#include <arch/i386/mem.h>

struct {
    uint32_t next_free_frame;
    multiboot_info_t *mboot_header;
    uint32_t mmap_length;
    multiboot_memory_map_t *mmap;
    multiboot_elf_section_header_table_t *elf_sec;
    uint32_t kernel_reserved_start;
    uint32_t kernel_reserved_end;
    uint32_t multiboot_reserved_start;
    uint32_t multiboot_reserved_end;
} info;

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

    // Fill local data structure with verified information
    info.next_free_frame = 1;
    info.mboot_header = mboot_header;
    info.mmap_length = mboot_header->mmap_length;
    info.mmap = (multiboot_memory_map_t *) mboot_header->mmap_addr;
    info.elf_sec = &(mboot_header->u.elf_sec);
    info.multiboot_reserved_start = (uint32_t)mboot_header;
    info.multiboot_reserved_end = (uint32_t)(mboot_header + sizeof(multiboot_info_t));

    // Parse ELF sections
    _i386_elf_sections_read();

    // Populate kernel_mem data struct with relevant variable and function pointers
    kernel_mem_info.next_free_frame = &info.next_free_frame;
    kernel_mem_info.get_frame_start_addr = &i386_mem_get_frame_start_addr;
    kernel_mem_info.get_frame_num = &i386_mem_get_frame_num;
    kernel_mem_info.allocate_frame = &i386_mem_allocate_frame;
    kernel_mem_info.kmalloc = &i386_mem_kmalloc;

    return;
multiboot_info_fail:
    printf("Multiboot information structure does not contain required sections!\n");
    abort();
}

/**
 * Allocate the next free frame and return the address it starts at
 * Returns frame number
 */
uint32_t i386_mem_allocate_frame() {
    uint32_t addr = _i386_mmap_read(info.next_free_frame, 0);

    // Check if all memory is allocated
    if (addr == 0) {
        printk_debug("Out of memory!");
        return 0;
    }

    // Check that frame is not in reserved area
    // If it is, recursively call back with an incremented frame number
    if (addr >= info.kernel_reserved_start && addr <= info.kernel_reserved_end) {
        info.next_free_frame++;
        return i386_mem_allocate_frame();
    } else if (addr >= info.multiboot_reserved_start && addr <= info.multiboot_reserved_end) {
        info.next_free_frame++;
        return i386_mem_allocate_frame();
    }

    // Update info.next_free_frame to reflect the latest frame.
    // We can't just increment by one because we may have skipped over multiple
    // chunks in reserved memory.
    uint32_t frame_num = i386_mem_get_frame_num(addr);
    info.next_free_frame = frame_num + 1;
    return frame_num;
}

/**
 * Return the address of the next free frame
 */
uint32_t i386_mem_get_next_frame() {
    return info.next_free_frame;
}

/**
 * Return the address of the next available frame after `counter`
 * Functions like i386_mem_allocate_frame, but doesn't increase internal frame
 * or allocate frame
 */
uint32_t i386_mem_peek_frame(uint32_t *counter) {
    uint32_t addr = _i386_mmap_read(*counter, 0);

    // Check if all memory is allocated
    if (addr == 0) {
        return 0;
    }

    // Check that frame is not in reserved area
    // If it is, recursively call back with an incremented frame number
    if (addr >= info.kernel_reserved_start && addr <= info.kernel_reserved_end) {
        (*counter)++;
        return i386_mem_peek_frame(counter);
    } else if (addr >= info.multiboot_reserved_start && addr <= info.multiboot_reserved_end) {
        (*counter)++;
        return i386_mem_peek_frame(counter);
    }

    // Update info.next_free_frame to reflect the latest frame.
    // We can't just increment by one because we may have skipped over multiple
    // chunks in reserved memory.
    uint32_t frame_num = i386_mem_get_frame_num(addr);
    *counter = frame_num + 1;
    return frame_num;
}

/**
 * Return frame start address for a given frame number, based on multiboot memory map.
 */
uint32_t i386_mem_get_frame_start_addr(uint32_t num) {
    // Get requested frame
    return _i386_mmap_read(num, 0);
}

/**
 * Return frame number for a given address, based on multiboot memory map
 */
 uint32_t i386_mem_get_frame_num(uint32_t addr) {
     return _i386_mmap_read(addr, 1);
 }

/**
 * Read multiboot memory map and return the start address of chunk `num`
 * NOTE: The very first chunk is ignored/reserved for kernel.
 * `mode` = 0 - Find chunk from num and return address
 * `mode` = 1 - Find chunk from address and return num (DEBUG)
 */
uint32_t _i386_mmap_read(uint32_t request, uint8_t mode) {
    //printk_debug("Parsing multiboot memory map.");

    // Skip requests for 0 or invalid mode
    if (request == 0 || (mode != 0 && mode != 1)) return 0;

    // Increment through each entry in the multiboot memory map
    uintptr_t cur_mmap_addr = (uintptr_t)info.mmap;
    uintptr_t mmap_end_addr = cur_mmap_addr + info.mmap_length;
    uint32_t cur_num = 0;
    while (cur_mmap_addr < mmap_end_addr) {
        multiboot_memory_map_t *current_entry = (multiboot_memory_map_t *)cur_mmap_addr;

        // Print out information for this entry
        //printf("[mem] addr: 0x%lx len: 0x%lx\n", current_entry->addr, current_entry->len);

        // Split this entry into PAGE_SIZE sized chunks and iterate through
        uint64_t i;
        uint64_t current_entry_end = current_entry->addr + current_entry->len;
        for (i=current_entry->addr; i + PAGE_SIZE <= current_entry_end; i += PAGE_SIZE) {
            if (mode == 1 && request >= i && request <= i+PAGE_SIZE) {
                return cur_num+1; // Return frame number
            }
            if (current_entry->type == MULTIBOOT_MEMORY_RESERVED) {
                // If the requested chunk is in reserved space, increase it until it is in non-reserved space
                if (mode == 0 && cur_num == request) {
                    ++request;
                }
                // Skip to next chunk
                ++cur_num;
                continue;
            } else if (mode == 0 && cur_num == request) {
                return i; // Return start address
            }
            ++cur_num;
        }

        // Increment by the size to get to the next entry
        cur_mmap_addr += current_entry->size + sizeof(uintptr_t);
    }

    // If no results are found, return 0
    return 0;
}

/**
 * Read ELF32 section headers and determine kernel reserved memory
 */
void _i386_elf_sections_read() {
    // Get first section headers
    elf_section_header_t *cur_header = (elf_section_header_t *)info.elf_sec->addr;

    // Print initial information about ELF section header availability
    //printf("[elf] First ELF section header at 0x%x, num_sections: 0x%x, size: 0x%x\n",
    //       info.elf_sec->addr, info.elf_sec->num, info.elf_sec->size);

    // Update local data to reflect reserved memory areas
    info.kernel_reserved_start = (cur_header + 1)->sh_addr;
    info.kernel_reserved_end = (cur_header + (info.elf_sec->num) - 1)->sh_addr;
}

/**
 * Allocate a continuous block of raw memory
 * @param size in bytes of block
 * @return starting address of block
 */
uint32_t i386_mem_kmalloc(uint32_t size) {
    // Determine number of pages we'll need to allocate to cover this size
    uint32_t frames = size / PAGE_SIZE;
    if (size % PAGE_SIZE != 0) ++frames;

    // Scan through frames until a continuous block of memory is found
    uint32_t mem_counter = 1;
    uint32_t start, end, target_frame;
    for (;;) {
        start = i386_mem_peek_frame(&mem_counter);
        // Fast forward `frames` frames and see if we're in the same block
        target_frame = start + frames - 1;
        end = i386_mem_peek_frame(&target_frame);

        if (end - start + 1 == frames) {
            // We have a block that starts and ends on available memory,
            // however, we must make sure all frames in between are available
            // too
            uint32_t i, temp;
            bool valid = true;
            for (i=start + 1; i<=end; i++) {
                // Ensure that this frame isn't reserved
                mem_counter = i;
                temp = i386_mem_peek_frame(&mem_counter);
                if (temp != i) {
                    // Frame is reserved, scrap block
                    valid = false;
                    break;
                }
            }
            if (valid) {
                return kernel_mem_get_frame_start_addr(start);
            } else {
                // We didn't find a continuous block, start looking again at
                // the first reserved frame
                mem_counter = temp;
            }
        } else {
            // We didn't find a continuous block, start looking again at `end`
            mem_counter = end;
        }
    }
    // No continuous chunks that big found
    return 0;
}
