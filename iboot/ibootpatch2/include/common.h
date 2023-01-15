#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>

#ifndef TESTBUILD
typedef uint64_t size_t;

#ifndef NULL
#define NULL ((void*)0)
#endif


// libc
void *memmem(const void *haystack, size_t hs_len, const void *needle, size_t ne_len);
void *memcpy(void *__restrict dst0, const void *__restrict src0, size_t len0);
void *memset(void *m, int c, size_t n);
int memcmp(const void *m1, const void *m2, size_t n);
#else
#include <stdio.h>
#include <strings.h>
#endif
#endif
