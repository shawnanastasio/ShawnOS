#pragma once

#include <stdint.h>
#include <string.h>

#include <kernel/kernel.h>

// Macros used in implementation
#define INDEX_FROM_BIT(a) (a / 32)
#define OFFSET_FROM_BIT(a) (a % 32)

struct bitset {
    uint32_t length; // Number of entries in bitset
    uint32_t *start; // Start address of bitset
};
typedef struct bitset bitset_t;

/**
 * Create a bitset
 * size must be cleanly divisible by sizeof(uint32_t)
 */
static inline void bitset_init(bitset_t *bitset, void *start, uint32_t length) {
    bitset->start = start;
    bitset->length = length;
    memset(start, 0, DIV_ROUND_UP(length, 32)*sizeof(uint32_t));
}

/**
 * Set a bit in a bitset
 * @param bitset pointer to bitset to act on
 * @param n      bit to set
 */
static inline void bitset_set_bit(bitset_t *bitset, uint32_t n) {
    ASSERT(bitset);
    ASSERT(n < bitset->length);
    bitset->start[INDEX_FROM_BIT(n)] |= (1 << OFFSET_FROM_BIT(n));
}

/**
 * Get the value of a bit in a bitset
 * @param  bitset pointer to bitset to act on
 * @param  n      bit to read
 * @return        value of requested bit
 */
static inline uint32_t bitset_get_bit(bitset_t *bitset, uint32_t n) {
    ASSERT(bitset);
    ASSERT(n < bitset->length);
    return bitset->start[INDEX_FROM_BIT(n)] & (1 << OFFSET_FROM_BIT(n));
}

/**
 * Clear a bit in a bitset
 * @param bitset pointer to bitset to act on
 * @param n      bit to clear
 */
static inline void bitset_clear_bit(bitset_t *bitset, uint32_t n) {
    ASSERT(bitset);
    ASSERT(n < bitset->length);
    bitset->start[INDEX_FROM_BIT(n)] &= ~(1 << OFFSET_FROM_BIT(n));
}
