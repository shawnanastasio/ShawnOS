#pragma once

#include <stdint.h>

#define PAGE_SIZE 4096

struct kernel_mem_allocator_data {
    uint32_t *next_free_frame;
    // Architecture-specific function to get frame start address from number.
    uint32_t (*get_frame_start_addr)(uint32_t);
    // Architecture-specific function to get frame number from any address in it.
    uint32_t (*get_frame_num)(uint32_t);
    // Architecture-specific function to allocate a new frame and return number.
    uint32_t (*allocate_frame)(void);
    // Architecture-specific function to allocate a continuous block of memory.
    uint32_t (*kmalloc)(uint32_t);
};
typedef struct kernel_mem_allocator_data kernel_mem_allocator_data_t;

extern kernel_mem_allocator_data_t kernel_mem_info;

uint32_t kernel_mem_allocate_frame();
uint32_t kernel_mem_get_frame_start_addr(uint32_t num);
uint32_t kernel_mem_get_frame_num(uint32_t addr);
uint32_t kernel_mem_kmalloc(uint32_t size);
