/**
 * ShawnOS kernel bitset implementation
 */

#include <stdint.h>

#include <kernel/kernel.h>
#include <kernel/bitset.h>

/**
 * Create a bitset
 */
void bitset_init(bitset_t *bitset, uint32_t *start, uint32_t size) {
    bitset->start = start;
    bitset->size = size;
}

/**
 * Set a bit in a bitset
 * @param bitset pointer to bitset to act on
 * @param n      bit to set
 */
inline void bitset_set_bit(bitset_t *bitset, uint32_t n) {
    ASSERT(n <= bitset->size, "kernel/bitset.c:bitset_set_bit():1 ASSERT failed!");
    ASSERT(bitset, "kernel/bitset.c:bitset_set_bit():2 ASSERT failed!");
    bitset->start[INDEX_FROM_BIT(n)] |= (1 << OFFSET_FROM_BIT(n));
}

/**
 * Get the value of a bit in a bitset
 * @param  bitset pointer to bitset to act on
 * @param  n      bit to read
 * @return        value of requested bit
 */
inline uint32_t bitset_get_bit(bitset_t *bitset, uint32_t n) {
    ASSERT(n <= bitset->size, "kernel/bitset.c:bitset_get_bit():1 ASSERT failed!");
    ASSERT(bitset, "kernel/bitset.c:bitset_get_bit():2 ASSERT failed!");
    return bitset->start[INDEX_FROM_BIT(n)] & (1 << OFFSET_FROM_BIT(n));
}

/**
 * Clear a bit in a bitset
 * @param bitset pointer to bitset to act on
 * @param n      bit to clear
 */
inline void bitset_clear_bit(bitset_t *bitset, uint32_t n) {
    ASSERT(n <= bitset->size, "kernel/bitset.c:bitset_clear_bit():1 ASSERT failed!");
    ASSERT(bitset, "kernel/bitset.c:bitset_clear_bit():2 ASSERT failed!");
    bitset->start[INDEX_FROM_BIT(n)] &= ~(1 << OFFSET_FROM_BIT(n));
}
