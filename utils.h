#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <stdlib.h>

// helpers to find the least common multiple.
#define MAX(a, b) a > b ? a : b;

// helpers to create a BLOCK_MASK with 'block_size' bits.
#define BLOCK_MSK(block_size) (uint64_t)((1 << block_size) - 1)
#define GEN_IV(block_size) rand_uint64() & BLOCK_MSK(block_size) 
#define GET_BLOCK_DATA(data, i, block_size) ((data & (BLOCK_MSK(block_size) << i)) >> i)

// helper to generate 64 bit unsigned.
uint64_t rand_uint64(void);

// fin the least common multiple.
uint64_t lcm(uint64_t a, uint64_t b);

// convert a binary code from string to real binary number!
// The sign is not that important here :)
int str2bin(const char *s, size_t key_len);

#endif