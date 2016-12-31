#pragma once

#include <stdint.h>

// Macros used in implementation
#define INDEX_FROM_BIT(a) (a / 32)
#define OFFSET_FROM_BIT(a) (a % 32)

struct bitset {
    uint32_t size;
    uint32_t *start;
};
typedef struct bitset bitset_t;

void bitset_init(bitset_t *bitset, uintptr_t *start, uint32_t size);
void bitset_set_bit(bitset_t *bitset, uint32_t n);
uint32_t bitset_get_bit(bitset_t *bitset, uint32_t n);
void bitset_clear_bit(bitset_t *bitset, uint32_t n);
