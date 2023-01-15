#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <printf.h>
#include <drivers/fb/fb.h>

//typedef uint64_t size_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

struct cmd_arg {
    bool b;
    size_t u;
    size_t h;
    char *str;
};

#define DT_KEY_LEN 0x20
#define BOOT_LINE_LENGTH_iOS12 0x100
#define BOOT_LINE_LENGTH_iOS13 0x260

struct Boot_Video {
    unsigned long    v_baseAddr;    /* Base address of video memory */
    unsigned long    v_display;    /* Display Code (if Applicable */
    unsigned long    v_rowBytes;    /* Number of bytes per pixel row */
    unsigned long    v_width;    /* Width */
    unsigned long    v_height;    /* Height */
    unsigned long    v_depth;    /* Pixel Depth and other parameters */
};
typedef struct boot_args {
    uint16_t        Revision;            /* Revision of boot_args structure */
    uint16_t        Version;            /* Version of boot_args structure */
    uint32_t        __pad0;
    uint64_t        virtBase;            /* Virtual base of memory */
    uint64_t        physBase;            /* Physical base of memory */
    uint64_t        memSize;            /* Size of memory */
    uint64_t        topOfKernelData;    /* Highest physical address used in kernel data area */
    struct Boot_Video Video;                /* Video Information */
    uint32_t        machineType;        /* Machine Type */
    uint32_t        __pad1;
    void            *deviceTreeP;        /* Base of flattened device tree */
    uint32_t        deviceTreeLength;    /* Length of flattened tree */
    
    char            CommandLine[BOOT_LINE_LENGTH_iOS13];    /* Passed in command line */
    uint32_t        __pad;
    uint64_t        bootFlags;        /* Additional flags specified by the bootloader */
    uint64_t        memSizeActual;        /* Actual size of memory */
    
} __attribute__((packed)) boot_args;

typedef struct
{
    uint32_t nprop;
    uint32_t nchld;
    char prop[];
} dt_node_t;

typedef struct
{
    char key[DT_KEY_LEN];
    uint32_t len;
    char val[];
} dt_prop_t;

struct memmap {
    uint64_t addr;
    uint64_t size;
};

extern void* gEntryPoint;
extern boot_args *gBootArgs;
extern dt_node_t *gDeviceTree;

extern int dt_check(void* mem, uint32_t size, uint32_t* offp);
extern int dt_parse(dt_node_t* node, int depth, uint32_t* offp, int (*cb_node)(void*, dt_node_t*), void* cbn_arg, int (*cb_prop)(void*, dt_node_t*, int, const char*, void*, uint32_t), void* cbp_arg);
extern dt_node_t* dt_find(dt_node_t* node, const char* name);
extern void* dt_prop(dt_node_t* node, const char* key, uint32_t* lenp);
extern void* dt_get_prop(const char* device, const char* prop, uint32_t* size);
extern struct memmap* dt_alloc_memmap(dt_node_t* node, const char* name);
extern uint64_t dt_get_u64_prop(const char* device, const char* prop);
extern uint64_t dt_get_u64_prop_i(const char* device, const char* prop, uint32_t idx);

// iboot
typedef int (*printf_t)(const char *format, ...);
printf_t iprintf;
typedef int (*jumpto_t)(void* arg0, void* arg1);
jumpto_t jumpto;
typedef void* (*malloc_t)(size_t size);
malloc_t imalloc;
typedef int (*panic_t)(const char *format, ...);
panic_t panic;
typedef void (*free_t)(void *ptr);
free_t ifree;


// libc
void *memmem(const void *haystack, size_t hs_len, const void *needle, size_t ne_len);
void *memcpy(void *__restrict dst0, const void *__restrict src0, size_t len0);
void *memset(void *m, int c, size_t n);
char *strchr (const char *s1, int i);
int strcmp (const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen (const char *str);
char *strcpy(char *dst0, const char *src0);
char *strstr (const char *haystack, const char *needle);
int memcmp(const void *m1, const void *m2, size_t n);
unsigned long long strtoull(const char * __restrict nptr, char ** __restrict endptr, int base);
char *strcat (char *__restrict s1, const char *__restrict s2);

#define DEV_BUILD 1

#ifdef DEV_BUILD
#define DEVLOG(x, ...) do { \
printf(x "\n", ##__VA_ARGS__); \
} while (0)
#define panic_at(addr, str, ...) do { \
panic(str " (0x%llx)", ##__VA_ARGS__, xnu_ptr_to_va(addr)); \
} while (0)
#else
#define DEVLOG(x, ...) do {} while (0)
#define panic_at(addr, str, ...) do { \
(void)(addr); \
panic(str, ##__VA_ARGS__); \
} while (0)
#endif

#define puts(x, ...) do { \
printf(x "\n", ##__VA_ARGS__); \
} while (0)

#endif
