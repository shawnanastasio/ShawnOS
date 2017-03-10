/**
 * ShawnOS kernel bitset implementation
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/kernel.h>
#include <kernel/bitset.h>

/**
 * Create a bitset
 * size must be cleanly divisible by sizeof(uint32_t)
 */
inline void bitset_init(bitset_t *bitset, uintptr_t *start, uint32_t length) {
    bitset->start = start;
    bitset->length = length;
    memset((void *)start, 0, DIV_ROUND_UP(length, 32)*sizeof(uint32_t));
}

/**
 * Set a bit in a bitset
 * @param bitset pointer to bitset to act on
 * @param n      bit to set
 */
inline void bitset_set_bit(bitset_t *bitset, uint32_t n) {
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
inline uint32_t bitset_get_bit(bitset_t *bitset, uint32_t n) {
    ASSERT(bitset);
    ASSERT(n < bitset->length);
    return bitset->start[INDEX_FROM_BIT(n)] & (1 << OFFSET_FROM_BIT(n));
}

/**
 * Clear a bit in a bitset
 * @param bitset pointer to bitset to act on
 * @param n      bit to clear
 */
inline void bitset_clear_bit(bitset_t *bitset, uint32_t n) {
    ASSERT(bitset);
    ASSERT(n < bitset->length);
    bitset->start[INDEX_FROM_BIT(n)] &= ~(1 << OFFSET_FROM_BIT(n));
}
