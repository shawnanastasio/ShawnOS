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
    uint32_t kernel_heap_start;
    uint32_t kernel_heap_end;
    uint32_t kernel_heap_curpos;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t highest_free_address;
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
    info.mem_upper = mboot_header->mem_upper;
    info.mem_lower = mboot_header->mem_lower;

    // Parse ELF sections
    _i386_elf_sections_read();

    // Find the highest free address
    info.highest_free_address = info.mem_upper * 1024;

    // Find block of memory to use as kernel heap
    info.kernel_heap_start = i386_mem_find_heap(KERNEL_HEAP_SIZE);
    // Check if heap was not found and handle accordingly
    if (info.kernel_heap_start == 0) {
        printf("Insufficient memory on device!\n");
        abort();
    }
    info.kernel_heap_curpos = info.kernel_heap_start;
    info.kernel_heap_end = info.kernel_heap_start + KERNEL_HEAP_SIZE;

    info.mem_lower = mboot_header->mem_lower;
    info.mem_upper = mboot_header->mem_upper;

    return;
multiboot_info_fail:
    printf("Multiboot information structure does not contain required sections!\n");
    abort();
}

/**
 * Allocate the next free frame and return its nu
 * Returns frame number
 */
uint32_t i386_mem_allocate_frame() {
    // Get address of next free frame
    uint32_t addr = info.next_free_frame * PAGE_SIZE;

    // Check if we've hit the end of available memory
    if (addr + PAGE_SIZE > info.highest_free_address) {
        printk_debug("Out of memory!");
        return 0;
    }

    // Check if the frame lies within reserved memory
    uint8_t reserved = i386_mem_check_reserved(addr);
    if (reserved == MEM_RESERVED) {
        printf("Reserved frame detected!!\n");
        // If it is, increment the frame number and recursively call back
        info.next_free_frame++;
        return i386_mem_allocate_frame();
    }

    // Frame is good, increment the next_free_frame and return this frame
    uint32_t temp = info.next_free_frame;
    info.next_free_frame++;
    return temp;
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
    // Get address of next free frame
    uint32_t addr = (*counter) * PAGE_SIZE;

    // Check if we've hit the end of available memory
    if (addr + PAGE_SIZE > info.highest_free_address) {
        return 0;
    }

    // Check if the frame lies within reserved memory
    uint8_t reserved = i386_mem_check_reserved(addr);
    if (reserved == MEM_RESERVED) {
        printf("Reserved frame detected!!\n");
        // If it is, increment the frame number and recursively call back
        (*counter)++;
        return i386_mem_peek_frame(counter);
    }

    // Frame is good, increment the next_free_frame and return this frame
    uint32_t temp = *counter;
    (*counter)++;
    return temp;
}

/**
 * Return frame start address for a given frame number, based on multiboot memory map.
 */
uint32_t i386_mem_get_frame_start_addr(uint32_t num) {
    // Get requested frame
    //return _i386_mmap_read(num, 0);
    return num*PAGE_SIZE;
}

/**
 * Return frame number for a given address, based on multiboot memory map
 */
uint32_t i386_mem_get_frame_num(uint32_t addr) {
    //return _i386_mmap_read(addr, 1);
    return addr/PAGE_SIZE;
}

/**
 * Checks if the frame at a given address is reserved in the memory map
 * @param  addr address of frame to check
 * @return reserved status
 */
uint8_t _i386_mmap_check_reserved(uint32_t addr) {
    uintptr_t cur_mmap_addr = (uintptr_t)info.mmap;
    uintptr_t mmap_end_addr = cur_mmap_addr + info.mmap_length;

    // Loop through memory map entries until the one for the requested addr is found
    while (cur_mmap_addr < mmap_end_addr) {
        multiboot_memory_map_t *current_entry = (multiboot_memory_map_t *)cur_mmap_addr;

        // Check if requested addr falls within the bounds of this entry
        uint32_t current_entry_end = current_entry->addr + current_entry->len;

        if (addr >= current_entry->addr && addr <= current_entry_end) {
            //printf("[mem] 0x%x found after 0x%x\n", addr, current_entry->addr);
            // Check if the entry marks this address as reserved
            if (current_entry->type == MULTIBOOT_MEMORY_RESERVED) {
                return MEM_RESERVED; // Memory is reserved
            } else {
                // Check if a page would fit within this entry at this address
                // If it doesn't, it means that part of the page falls within
                // reserved memory.
                if (addr + PAGE_SIZE > current_entry_end) {
                    return MEM_RESERVED;
                } else {
                    return MEM_FREE; // Memory is not reserved
                }
            }
        }

        cur_mmap_addr += current_entry->size + sizeof(uintptr_t);
    }

    // If the memory address was not found in the multiboot memory map, return
    // false.
    printk_debug("Requested address not found in memory map!");
    printf("Requested addr was 0x%x\n", addr);
    return MEM_NONEXISTANT;
}

/**
 * Debug function to print out reserved memory regions to the screen
 */
void _i386_print_reserved() {
    // Print out mem_lower and mem_upper
    printf("[mem] mem_lower: %u, mem_upper: %u, mem_total: %u\n",
            info.mem_lower, info.mem_upper, info.mem_lower + info.mem_upper);
    // Print out mmap
    uintptr_t cur_mmap_addr = (uintptr_t)info.mmap;
    uintptr_t mmap_end_addr = cur_mmap_addr + info.mmap_length;

    // Loop through memory map and print entries
    while (cur_mmap_addr < mmap_end_addr) {
        multiboot_memory_map_t *current_entry = (multiboot_memory_map_t *)cur_mmap_addr;

        printf("[mem] addr: 0x%lx len: 0x%lx reserved: %u\n", current_entry->addr,
                current_entry->len, current_entry->type);

        cur_mmap_addr += current_entry->size + sizeof(uintptr_t);
    }

    // Print out ELF
    printf("[mem] elf reserved start: 0x%x end: 0x%x\n",
            info.kernel_reserved_start, info.kernel_reserved_end);

    // Print out mboot reserved
    printf("[mem] mboot reserved start: 0x%x end: 0x%x\n",
            info.multiboot_reserved_start, info.multiboot_reserved_end);
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
 * Find a continuous block of memory with size `size` to be used as kernel heap
 * Memory is page aligned.
 * @param size in bytes of heap
 * @return starting address of heap
 */
uint32_t i386_mem_find_heap(uint32_t size) {
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

        //printf("end: %u, start: %u, frames: %u\n", end, start, frames);
        if (end - start + 1 == frames) {
            // We have a block that starts and ends on available memory,
            // however, we must make sure all frames in between are available
            // too
            uint32_t i, temp;
            bool valid = true;
            for (i=start + 1; i<end; i++) {
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
                return i386_mem_get_frame_start_addr(start);
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

/**
 * Checks if the frame at memory address `addr` is in any reserved memory
 * @return true if memory is reserved, otherwise false
 */
uint8_t i386_mem_check_reserved(uint32_t addr) {
    // End of the page that starts at addr
    uint32_t addr_end = addr + PAGE_SIZE;

    // Check that memory doesn't overlap with kernel binary
    if ((addr >= info.kernel_reserved_start && addr <= info.kernel_reserved_end) ||
        (addr_end >= info.kernel_reserved_start && addr_end <= info.kernel_reserved_end)) {
        return MEM_RESERVED;
    }

    // Check that memory doesn't overlap with multiboot information structure
    if ((addr >= info.multiboot_reserved_start && addr <= info.multiboot_reserved_end) ||
        (addr_end >= info.multiboot_reserved_start && addr_end <=info.multiboot_reserved_end)) {
        return MEM_RESERVED;
    }

    // Check that the memory isn't marked as reserved in the memory map
    uint8_t mmap_reserved = _i386_mmap_check_reserved(addr);
    if (mmap_reserved != MEM_FREE) {
        return mmap_reserved;
    }

    // If we got here, it means the memory is not reserved
    return MEM_FREE;
}

/**
 * Internal function to allocate memory from the kernel heap. Should not be called
 * directly.
 *
 * @param size size of memory to allocate
 * @param align whether or not to align memory to page bounds
 * @param physical address of allocated memory
 */
uintptr_t i386_mem_kmalloc_real(uint32_t size, bool align, uintptr_t *phys) {
    // Page-align address if requested
    if (align == true && (info.kernel_heap_curpos % PAGE_SIZE != 0)) {
        // The address is not already aligned, so we must do it ourselves
        info.kernel_heap_curpos &= 0x100000000 - PAGE_SIZE;
        info.kernel_heap_curpos += PAGE_SIZE;
    }
    // If page-alignment isn't requested, make the address 8-byte aligned.
    // 8-byte is chosen as a common value that doesn't clash with most C datatypes'
    // natural alignments
    else if (align == false && (info.kernel_heap_curpos % 0x8 != 0)) {
        info.kernel_heap_curpos += info.kernel_heap_curpos % 0x8;
    }
    if (phys != NULL) {
        // If a physical address pointer is provided to us, update it
        *phys = (uintptr_t)info.kernel_heap_curpos;
    }
    // Increase the current position past the allocated memory
    uint32_t allocated_memory_start = info.kernel_heap_curpos;
    info.kernel_heap_curpos += size;
    return allocated_memory_start;
}

/**
 * Allocate a standard chunk of memory in the kernel heap
 *
 * @param size size of memory to allocate
 */
uintptr_t i386_mem_kmalloc(uint32_t size) {
    return i386_mem_kmalloc_real(size, false, NULL);
}

/**
 * Allocate a page-aligned chunk of memory in the kernel heap
 *
 * @param size size of memory to allocate
 */
uintptr_t i386_mem_kmalloc_a(uint32_t size) {
    return i386_mem_kmalloc_real(size, true, NULL);
}

/**
 * Allocate a standard chunk of memory in the kernel heap and return
 * the physical address that it is located at
 *
 * @param size size of memory to allocate
 * @param phys pointer to uintptr_t to store physical address at
 */
uintptr_t i386_mem_kmalloc_p(uint32_t size, uintptr_t *phys) {
    return i386_mem_kmalloc_real(size, false, phys);
}

/**
 * Allocate a page-aligned chunk of memory in the kernel heap and return
 * the physical address that it is located at
 *
 * @param size size of memory to allocate
 * @param phys pointer to uintptr_t to store physical address at
 */
uintptr_t i386_mem_kmalloc_ap(uint32_t size, uintptr_t *phys) {
    return i386_mem_kmalloc_real(size, true, phys);
}
