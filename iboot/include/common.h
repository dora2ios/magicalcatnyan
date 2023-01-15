#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t size_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

// iboot
typedef int (*write_t)(uint64_t arg0);
write_t heapWriteHash;
typedef int (*check_t)(uint64_t arg0);
check_t heapCheckAll;

typedef void (*memcpy_t)(void *__restrict dst0, const void *__restrict src0, size_t len0);
memcpy_t my_memcpy;

#endif
