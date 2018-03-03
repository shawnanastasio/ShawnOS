#pragma once

#include <stdint.h>
#include <stddef.h>

#include <kernel/kernel.h>
#include <kernel/bitset.h>

typedef struct kasa_data {
    bitset_t pages;
    uint32_t page_size;
} kasa_data_t;

k_return_t asa_init();
void *asa_alloc(uint32_t n_pages);
k_return_t asa_free(void *addr, uint32_t n_pages);