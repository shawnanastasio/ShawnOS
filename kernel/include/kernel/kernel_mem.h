#pragma once

/**
 * This file includes architecture-specific aliases for kernel memory allocation
 * functions.
 */

// i386 memory aliases
// TODO: Conditionally include these based on preprocessor definitions
#define kmalloc_real(...) i386_mem_kmalloc_real(__VA_ARGS__)
#define kmalloc(...) i386_mem_kmalloc(__VA_ARGS__)
#define kmalloc_a(...) i386_mem_kmalloc_a(__VA_ARGS__)
#define kmalloc_p(...) i386_mem_kmalloc_p(__VA_ARGS__)
#define kmalloc_ap(...) i386_mem_kmalloc_ap(__VA_ARGS__)
