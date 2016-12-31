#pragma once

#include <stdint.h>
#include <stddef.h>

#include <kernel/bitset.h>

#define BLOCK_SIZE_DEFAULT 0x10000 // Each heap entry (block) is 0x10000 bytes
#define SECTION_SIZE_DEFAULT 0x10  // Each block is sectioned into 16byte chunks

#define SECTION_MAGIC 0xDEADBEEF

/**
 * Struct placed at the beginning of each heap block
 * Serves as a link list of blocks
 */
struct kheap_block {
    struct kheap_block *next;
    uint32_t block_size;
    uint32_t section_size;
    uintptr_t start;
    bitset_t bitset;
};
typedef struct kheap_block kheap_block_t;

/**
 * Struct placed at beginning of each allocated region
 * Stores metadata used when freeing regions
 */
struct kheap_alloc_header {
    uint32_t magic; // Magic number used for verification
    uint32_t size;  // Full size of section including this header
};
typedef struct kheap_alloc_header kheap_alloc_header_t;

/**
 * Struct for storing heap metadata
 * Acted upon by all kheap functions
 */
struct kheap {
    kheap_block_t *first; // First block in kernel heap
                          // new blocks will go here
};
typedef struct kheap kheap_t;


void kheap_init(kheap_t *heap);
void kheap_add_block(kheap_t *heap, uintptr_t *start, uint32_t block_size,
                     uint32_t section_size);
uintptr_t kheap_malloc(kheap_t *heap, size_t size);
void kheap_free(kheap_t *heap, uintptr_t addr);
