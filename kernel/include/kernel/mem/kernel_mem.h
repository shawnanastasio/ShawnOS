#pragma once

#include <stdint.h>
#include <arch/i386/multiboot.h>

void kernel_mem_mmap_read(uint32_t mmap_length, multiboot_memory_map_t *mmap);
