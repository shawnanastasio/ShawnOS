/**
 * ShawnOS kernel bitset implementation
 */

#include <stdint.h>
#include <string.h>

#include <kernel/kernel.h>
#include <kernel/bitset.h>

/**
 * Create a bitset
 * size must be cleanly divisible by sizeof(uint32_t)
 */
inline void bitset_init(bitset_t *bitset, uintptr_t *start, uint32_t size) {
    bitset->start = start;
    bitset->size = size;
    memset((void *)start, 0, bitset->size);
}

/**
 * Set a bit in a bitset
 * @param bitset pointer to bitset to act on
 * @param n      bit to set
 */
inline void bitset_set_bit(bitset_t *bitset, uint32_t n) {
    ASSERT(n < bitset->size);
    ASSERT(bitset);
    bitset->start[INDEX_FROM_BIT(n)] |= (1 << OFFSET_FROM_BIT(n));
}

/**
 * Get the value of a bit in a bitset
 * @param  bitset pointer to bitset to act on
 * @param  n      bit to read
 * @return        value of requested bit
 */
inline uint32_t bitset_get_bit(bitset_t *bitset, uint32_t n) {
    ASSERT(n < bitset->size);
    ASSERT(bitset);
    return bitset->start[INDEX_FROM_BIT(n)] & (1 << OFFSET_FROM_BIT(n));
}

/**
 * Clear a bit in a bitset
 * @param bitset pointer to bitset to act on
 * @param n      bit to clear
 */
inline void bitset_clear_bit(bitset_t *bitset, uint32_t n) {
    ASSERT(n < bitset->size);
    ASSERT(bitset);
    bitset->start[INDEX_FROM_BIT(n)] &= ~(1 << OFFSET_FROM_BIT(n));
}
