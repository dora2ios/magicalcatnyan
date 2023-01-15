#ifndef OFFSETFINDER_H
#define OFFSETFINDER_H

#include <stdint.h>
#include <common.h>

uint64_t find_printf(uint64_t region, uint8_t* data, size_t size);
uint64_t find_jumpto_func(uint64_t region, uint8_t* data, size_t size);
uint64_t find_malloc(uint64_t region, uint8_t* data, size_t size);
uint64_t find_panic(uint64_t region, uint8_t* data, size_t size);
uint64_t find_free(uint64_t region, uint8_t* data, size_t size);

#endif
