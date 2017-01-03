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
    /**
     * Bitset containing the free/allocated status of each section in the block
     */
    bitset_t used_sections;
    /**
     * Bitset containing delimiter information for each section in the block
     * A section is marked as a delimiter if it is the last section in an
     * allocation that has not yet been freed.
     */
    bitset_t delimiters;
};
typedef struct kheap_block kheap_block_t;

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
void _kheap_print(kheap_t *heap);
void kheap_expand(kheap_t *heap, size_t size);
void kheap_add_block(kheap_t *heap, uintptr_t *start, uint32_t block_size,
                     uint32_t section_size);
uintptr_t kheap_malloc(kheap_t *heap, size_t size);
void kheap_free(kheap_t *heap, uintptr_t addr);
