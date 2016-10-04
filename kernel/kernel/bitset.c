/**
 * ShawnOS kernel bitset implementation
 */

#include <stdint.h>

#include <kernel/bitset.h>

/**
 * Set a bit in a bitset
 * @param bitset pointer to bitset to act on
 * @param n      bit to set
 */
inline void bitset_set_bit(uint32_t *bitset, uint32_t n) {
    bitset[INDEX_FROM_BIT(n)] |= (1 << OFFSET_FROM_BIT(n));
}

/**
 * Get the value of a bit in a bitset
 * @param  bitset pointer to bitset to act on
 * @param  n      bit to read
 * @return        value of requested bit
 */
inline uint32_t bitset_get_bit(uint32_t *bitset, uint32_t n) {
    return bitset[INDEX_FROM_BIT(n)] & (1 << OFFSET_FROM_BIT(n));
}

/**
 * Clear a bit in a bitset
 * @param bitset pointer to bitset to act on
 * @param n      bit to clear
 */
inline void bitset_clear_bit(uint32_t *bitset, uint32_t n) {
    bitset[INDEX_FROM_BIT(n)] &= ~(1 << OFFSET_FROM_BIT(n));
}
