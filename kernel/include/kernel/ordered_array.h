#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * Function to compare two objects
 * @param a first object to compare
 * @param b second object to compare
 * @return negative int if a < b, 0 if a == b, positive int if a > b
 */
typedef int32_t (*comparator_t)(void *a, void *b);

struct ordered_array {
   void **array;
   uint32_t size;
   uint32_t max_size;
   comparator_t comparator;
};
typedef struct ordered_array ordered_array_t;

int32_t comparator_default(void *a, void *b);
ordered_array_t ordered_array_create(void *addr, uint32_t max_size, comparator_t comparator);
bool ordered_array_insert(ordered_array_t *array, void *object);
void *ordered_array_get(ordered_array_t *array, uint32_t index);
bool ordered_array_remove(ordered_array_t *array, uint32_t index);
