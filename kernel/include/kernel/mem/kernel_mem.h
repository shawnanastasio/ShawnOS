#pragma once

#include <stdint.h>
#include <arch/i386/multiboot.h>

void kernel_mem_mmap_read(mboot_memmap_t *mmap);
