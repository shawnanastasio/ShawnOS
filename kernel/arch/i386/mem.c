/**
 * @file
 * @brief Architecture-specific memory allocation functions
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <kernel/kernel.h>
#include <kernel/kernel_mem.h>
#include <kernel/kernel_thread.h>
#include <kernel/bitset.h>

#include <arch/i386/multiboot.h>
#include <arch/i386/elf.h>
#include <arch/i386/mem.h>
#include <arch/i386/paging.h>

// Global pointer to a bitset containing reserved frames
uint32_t *i386_mem_frame_bitset;

i386_mem_info_t meminfo;

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
    meminfo.mboot_header = mboot_header;
    meminfo.mmap_length = mboot_header->mmap_length;
    meminfo.mmap = (multiboot_memory_map_t *) mboot_header->mmap_addr;
    meminfo.elf_sec = &(mboot_header->u.elf_sec);
    meminfo.multiboot_reserved_start = (uint32_t)mboot_header;
    meminfo.multiboot_reserved_end = (uint32_t)(mboot_header + sizeof(multiboot_info_t));
    meminfo.mem_upper = mboot_header->mem_upper;
    meminfo.mem_lower = mboot_header->mem_lower;

    // Parse ELF sections
    _i386_elf_sections_read();

    // Find the highest free address
    meminfo.highest_free_address = meminfo.mem_upper * 1024;

    // Set the kernel heap size to the amount of memory required for the frame bitmap
    //meminfo.kernel_heap_size = meminfo.highest_free_address/PAGE_SIZE;
    meminfo.kernel_heap_size = 0x30000;

    // Find block of memory to use as kernel heap
    meminfo.kernel_heap_start = i386_mem_find_heap(meminfo.kernel_heap_size);
    // Check if heap was not found and handle accordingly
    if (meminfo.kernel_heap_start == 0) {
        printf("Insufficient memory on device!\n");
        abort();
    }
    meminfo.kernel_heap_curpos = meminfo.kernel_heap_start;
    meminfo.kernel_heap_end = meminfo.kernel_heap_start + meminfo.kernel_heap_size;

    // Allocate required memory for mem frame bitset and mark reserved frames
    i386_mem_frame_bitset = (uint32_t *)kmalloc_a(meminfo.highest_free_address/PAGE_SIZE);
    memset((void *)i386_mem_frame_bitset, 0, meminfo.highest_free_address/PAGE_SIZE);
    _i386_mem_init_bitset();

    return;
multiboot_info_fail:
    printf("Multiboot information structure does not contain required sections!\n");
    abort();
}

/**
 * Initialize the bitset containing reserved frames
 */
void _i386_mem_init_bitset() {
    uint32_t i;

    // Iterate through memory and mark reserved frames in bitset
    for (i = 0x0000; i + PAGE_SIZE < meminfo.highest_free_address; i += PAGE_SIZE) {
        // Check if the current address is reserved
        if (i386_mem_check_reserved(i) == MEM_RESERVED) {
            // Mark frame as reserved in the bitset
            bitset_set_bit(i386_mem_frame_bitset, i / PAGE_SIZE);
        }
    }

    // Mark kernel heap memory as reserved
    uint32_t start = (uint32_t)(meminfo.kernel_heap_start / PAGE_SIZE);
    uint32_t end = (uint32_t)(meminfo.kernel_heap_end / PAGE_SIZE);
    for (i=start; i<=end; i++) {
        bitset_set_bit(i386_mem_frame_bitset, i);
    }
}

/**
 * Allocates a frame and returns it's index
 * @return index of allocated frame
 */
inline uint32_t i386_mem_allocate_frame() {
    uint32_t next_free_frame = i386_mem_get_next_free_frame();
    if (next_free_frame) {
        bitset_set_bit(i386_mem_frame_bitset, next_free_frame);
    }
    return next_free_frame;
}

/**
 * Frees a frame
 * @param frame index of frame to free
 */
inline void i386_mem_free_frame(uint32_t frame) {
    bitset_clear_bit(i386_mem_frame_bitset, frame);
}

/**
 * Get the frame number of the next available frame
 * @return frame number
 */
uint32_t i386_mem_get_next_free_frame() {
    uint32_t i, j;

    // Loop through bitset until a free frame is found
    for (i=0; i < INDEX_FROM_BIT(meminfo.highest_free_address/PAGE_SIZE); i++) {
        // Skip this entry if it is full
        if (i386_mem_frame_bitset[i] == 0xFFFFFFFF) continue;
        //printf("Current Entry: 0x%x\n", i386_mem_frame_bitset[i]);

        // Check if this entry has a free frame
        for (j=0; j < 32; j++) {
            if (!(i386_mem_frame_bitset[i] & (1 << j))) {
                // This frame is free, return it
                //printf("FRAME #%u is free, returning!\n", i * 32 + j);
                //printf("bitwise result: %u\n", i386_mem_frame_bitset[i] & (1 < j));
                return i * 32 + j;
            }
        }
    }

    // No free frames were found
    printk_debug("Out of memory!");
    return 0;
}

/**
 * Find the first free frame at or after the provided counter
 * Provided for legacy support with previous memory function implementations
 * @param  counter pointer to the counter to use
 * @return         frame number
 */
uint32_t i386_mem_peek_frame(uint32_t *counter) {
    // Get address of next free frame
    uint32_t addr = (*counter) * PAGE_SIZE;

    // Check if we've hit the end of available memory
    if (addr + PAGE_SIZE > meminfo.highest_free_address) {
        return 0;
    }

    // Check if the frame lies within reserved memory
    uint8_t reserved = i386_mem_check_reserved(addr);
    if (reserved == MEM_RESERVED) {
        // If it is, increment the frame number and recursively call back
        (*counter)++;
        return i386_mem_peek_frame(counter);
    }

    // Frame is good, increment the counter and return this frame
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
    uintptr_t cur_mmap_addr = (uintptr_t)meminfo.mmap;
    uintptr_t mmap_end_addr = cur_mmap_addr + meminfo.mmap_length;

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
    //printk_debug("Requested address not found in memory map!");
    //printf("Requested addr was 0x%x\n", addr);
    return MEM_NONEXISTANT;
}

/**
 * Debug function to print out reserved memory regions to the screen
 */
void _i386_print_reserved() {
    // Print out mem_lower and mem_upper
    printf("[mem] mem_lower: %u, mem_upper: %u, mem_total: %u\n",
            meminfo.mem_lower, meminfo.mem_upper, meminfo.mem_lower + meminfo.mem_upper);
    // Print out mmap
    uintptr_t cur_mmap_addr = (uintptr_t)meminfo.mmap;
    uintptr_t mmap_end_addr = cur_mmap_addr + meminfo.mmap_length;

    // Loop through memory map and print entries
    while (cur_mmap_addr < mmap_end_addr) {
        multiboot_memory_map_t *current_entry = (multiboot_memory_map_t *)cur_mmap_addr;

        printf("[mem] addr: 0x%lx len: 0x%lx reserved: %u\n", current_entry->addr,
                current_entry->len, current_entry->type);

        cur_mmap_addr += current_entry->size + sizeof(uintptr_t);
    }

    // Print out ELF
    printf("[mem] elf reserved start: 0x%x end: 0x%x\n",
            meminfo.kernel_reserved_start, meminfo.kernel_reserved_end);

    // Print out mboot reserved
    printf("[mem] mboot reserved start: 0x%x end: 0x%x\n",
            meminfo.multiboot_reserved_start, meminfo.multiboot_reserved_end);
}

/**
 * Read ELF32 section headers and determine kernel reserved memory
 */
void _i386_elf_sections_read() {
    // Get first section headers
    elf_section_header_t *cur_header = (elf_section_header_t *)meminfo.elf_sec->addr;

    // Print initial information about ELF section header availability
    //printf("[elf] First ELF section header at 0x%x, num_sections: 0x%x, size: 0x%x\n",
    //       meminfo.elf_sec->addr, meminfo.elf_sec->num, meminfo.elf_sec->size);

    // Update local data to reflect reserved memory areas
    meminfo.kernel_reserved_start = (cur_header + 1)->sh_addr;
    meminfo.kernel_reserved_end = (cur_header + (meminfo.elf_sec->num) - 1)->sh_addr;
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

    // Below 0x1000 is always reserved
    if (addr_end <= 0x1000) return MEM_RESERVED;

    // Check that memory doesn't overlap with kernel binary
    if ((addr >= meminfo.kernel_reserved_start && addr <= meminfo.kernel_reserved_end) ||
        (addr_end >= meminfo.kernel_reserved_start && addr_end <= meminfo.kernel_reserved_end)) {
        return MEM_RESERVED;
    }

    // Check that memory doesn't overlap with multiboot information structure
    if ((addr >= meminfo.multiboot_reserved_start && addr <= meminfo.multiboot_reserved_end) ||
        (addr_end >= meminfo.multiboot_reserved_start && addr_end <=meminfo.multiboot_reserved_end)) {
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
    if (align == true && (meminfo.kernel_heap_curpos % PAGE_SIZE != 0)) {
        // The address is not already aligned, so we must do it ourselves
        meminfo.kernel_heap_curpos &= 0x100000000 - PAGE_SIZE;
        meminfo.kernel_heap_curpos += PAGE_SIZE;
    }
    // If page-alignment isn't requested, make the address 8-byte aligned.
    // 8-byte is chosen as a common value that doesn't clash with most C datatypes'
    // natural alignments
    else if (align == false && (meminfo.kernel_heap_curpos % 0x8 != 0)) {
        meminfo.kernel_heap_curpos += meminfo.kernel_heap_curpos % 0x8;
    }
    if (phys != NULL) {
        // If a physical address pointer is provided to us, update it
        *phys = (uintptr_t)meminfo.kernel_heap_curpos;
    }
    // Increase the current position past the allocated memory
    uint32_t allocated_memory_start = meminfo.kernel_heap_curpos;
    meminfo.kernel_heap_curpos += size;
    return allocated_memory_start;
}

/**
 * Allocate a standard chunk of memory in the kernel heap
 *
 * @param size size of memory to allocate
 */
inline uintptr_t i386_mem_kmalloc(uint32_t size) {
    return i386_mem_kmalloc_real(size, false, NULL);
}

/**
 * Allocate a page-aligned chunk of memory in the kernel heap
 *
 * @param size size of memory to allocate
 */
inline uintptr_t i386_mem_kmalloc_a(uint32_t size) {
    return i386_mem_kmalloc_real(size, true, NULL);
}

/**
 * Allocate a standard chunk of memory in the kernel heap and return
 * the physical address that it is located at
 *
 * @param size size of memory to allocate
 * @param phys pointer to uintptr_t to store physical address at
 */
inline uintptr_t i386_mem_kmalloc_p(uint32_t size, uintptr_t *phys) {
    return i386_mem_kmalloc_real(size, false, phys);
}

/**
 * Allocate a page-aligned chunk of memory in the kernel heap and return
 * the physical address that it is located at
 *
 * @param size size of memory to allocate
 * @param phys pointer to uintptr_t to store physical address at
 */
inline uintptr_t i386_mem_kmalloc_ap(uint32_t size, uintptr_t *phys) {
    return i386_mem_kmalloc_real(size, true, phys);
}
