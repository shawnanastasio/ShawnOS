#pragma once

#include <stdint.h>

// Macros used in implementation
#define INDEX_FROM_BIT(a) (a / 32)
#define OFFSET_FROM_BIT(a) (a % 32)

void bitset_set_bit(uint32_t *bitset, uint32_t n);
uint32_t bitset_get_bit(uint32_t *bitset, uint32_t n);
void bitset_clear_bit(uint32_t *bitset, uint32_t n);
