/**
 * Kernel ordered array implementation
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <kernel/ordered_array.h>

/**
 * The default comparator
 * Compares addresses of pointer
 * @param  a first object to compare
 * @param  b second object to compare
 * @return   negative int if a < b, 0 if a == b, positive int if a > b
 */
inline int32_t comparator_default(void *a, void *b) {
    return (int32_t)((uintptr_t)a - (uintptr_t)b);
}

/**
 * Function to create an ordered list
 * @param  addr      address to place array at
 * @param  max_size  maximum size of the array
 * @param  less_than custom comparator function, or NULL to use default
 * @return           ordered array struct
 */
ordered_array_t ordered_array_create(void *addr, uint32_t max_size, comparator_t comparator) {
    ordered_array_t new_array;
    new_array.array = (void **)addr;
    new_array.max_size = max_size;
    if (comparator)
        new_array.comparator = comparator;
    else
        new_array.comparator = comparator_default;

    memset(new_array.array, 0, max_size * sizeof(void *));
    return new_array;
}

/**
 * Insert an object into an ordered array
 * @param array ordered array to act on
 * @param object  object to insert
 * @return        true if success, otherwise false
 */
bool ordered_array_insert(ordered_array_t *array, void *object) {
    // Check if we have room
    if (array->size >= array->max_size)
        return false;

    uint32_t i = 0;
    for (i=0; i<array->size; i++) {
        // Loop through until we find an item that is greater than our object
        if (array->comparator(array->array[i], object) >= 0)
            break;
    }

    // Insert the object at the index we just got
    if (i != array->size) {
        // Insert in array at index
        // Shift every object at or above the index up one
        uint32_t j;
        for (j=array->size-1; j <= i; j--) {
            array->array[j+1] = array->array[j];
        }
        array->array[i] = object;
        array->size++;
        return true;
    } else {
        // Add to the end of the array
        array->array[array->size] = object;
        array->size++;
        return true;
    }

}

/**
 * Get an object by index in an ordered array
 * @param  array ordered array to act on
 * @param  index index of object to retrieve
 * @return       object at specified index
 */
inline void *ordered_array_get(ordered_array_t *array, uint32_t index) {
    if (index > array->size) return NULL;
    return array->array[index];
}

/**
 * Delete an object at specified index in an ordered array
 * @param array ordered array to act on
 * @param index index of object to remove
 * @return      true on success, otherwise false
 */
bool ordered_array_remove(ordered_array_t *array, uint32_t index) {
    if (index > array->size) return false;
    // Shift everything above index down one
    while (index < array->size) {
        array->array[index] = array->array[index+1];
        ++index;
    }

    // Lower size of array by one
    array->size = array->size - 1;
    return true;
}
