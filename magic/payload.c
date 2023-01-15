#include <stdint.h>
#include <stdbool.h>

#include <common.h>
#include <offsetfinder.h>
#include <drivers/fb/fb.h>

void kpf(void);

int payload(int argc, struct cmd_arg *args)
{
     
    return 0;
}


boot_args *gBootArgs;
void *gEntryPoint;
dt_node_t *gDeviceTree;
uint64_t gIOBase;
uint64_t gWDTBase;

#define WDT_CHIP_TMR (*(volatile uint32_t*)(gWDTBase + 0x0))
#define WDT_CHIP_RST (*(volatile uint32_t*)(gWDTBase + 0x4))
#define WDT_CHIP_INT (*(volatile uint32_t*)(gWDTBase + 0x8))
#define WDT_CHIP_CTL (*(volatile uint32_t*)(gWDTBase + 0xc))

#define WDT_SYS_TMR (*(volatile uint32_t*)(gWDTBase + 0x10))
#define WDT_SYS_RST (*(volatile uint32_t*)(gWDTBase + 0x14))
#define WDT_SYS_CTL (*(volatile uint32_t*)(gWDTBase + 0x1c))

uint64_t ttb_alloc_base;
uint32_t* ppage_list;
uint64_t ppages = 0;
uint64_t free_pages = 0;
uint64_t wired_pages = 0;

#define PAGE_FREE 0
#define PAGE_WIRED 0xffffff
#define PAGE_REFBITS 0xffffff

static uint64_t iboot_base;

static int iboot_init(uint16_t cpid)
{
    if(cpid == 0x8015)
        iboot_base = 0x18001c000;
    else if(cpid == 0x8011)
        iboot_base = 0x1800b0000;
    else if(cpid == 0x8010)
        iboot_base = 0x1800b0000;
    else 
        return -1;
    
    uint8_t* idata = (uint8_t*)(iboot_base);
    size_t isize = *(uint64_t *)(idata + 0x308) - iboot_base;
    
    /*-- offsetfinder --*/
    uint64_t func_printf = find_printf(iboot_base, idata, isize);
    if(!func_printf)
        return -1;
    func_printf += iboot_base;
    
    uint64_t func_jumpto = find_jumpto_func(iboot_base, idata, isize);
    if(!func_jumpto)
        return -1;
    func_jumpto += iboot_base;
    
    uint64_t func_malloc = find_malloc(iboot_base, idata, isize);
    if(!func_malloc)
        return -1;
    func_malloc += iboot_base;
    
    uint64_t func_panic = find_panic(iboot_base, idata, isize);
    if(!func_panic)
        return -1;
    func_panic += iboot_base;
    
    uint64_t func_free = find_free(iboot_base, idata, isize);
    if(!func_free)
        return -1;
    func_free += iboot_base;
    
    iprintf = (printf_t)func_printf;
    jumpto  = (jumpto_t)func_jumpto;
    imalloc = (malloc_t)func_malloc;
    panic   = (panic_t)func_panic;
    ifree   = (free_t)func_free;
    
    return 0;
}


extern void enable_interrupts();
extern void disable_interrupts();
uint64_t pa_head;
// hacky haxx
uint64_t vatophys_static(void* kva) {
    return (((uint64_t)kva));
}

void* phystokv(uint64_t paddr) {
    return (void*)(paddr);
}

void phys_page_was_freed(uint64_t pa) {
    disable_interrupts();
    uint64_t* pa_v = phystokv(pa);
    if (pa_head) {
        uint64_t* pa_head_v = phystokv(pa_head);
        pa_head_v[1] = pa; // head->prev = new
    }
    pa_v[0] = pa_head; // new->next = head
    pa_v[1] = 0; // new->prev == null
    pa_head = pa; // head = new
    free_pages ++;
    enable_interrupts();
}

void phys_force_free(uint64_t pa, uint64_t size) {
    pa -= gBootArgs->physBase;
    
    uint64_t fpages = size >> 14;
    if (pa & 0x3fff) panic("phys_force_free only works with aligned PAs");
    pa >>= 14;
    
    disable_interrupts();
    for (uint64_t i=pa; i < pa+fpages; i++) {
        if (i > ppages) panic("OOB phys_force_free: 0x%llx", i << 14ULL);
        if ((ppage_list[i] & PAGE_REFBITS) == PAGE_WIRED) {
            wired_pages--;
        }
        if ((ppage_list[i] & PAGE_REFBITS) != PAGE_FREE) {
            phys_page_was_freed((i << 14ULL) + gBootArgs->physBase);
        }
        ppage_list[i] = PAGE_FREE;
    }
    enable_interrupts();
}

uint64_t overlay_base_address;
extern uint64_t OVERLAY_DATA;
extern uint64_t OVERLAY_SIZE;

void payload_entry(uint64_t *kernel_args, void *entryp)
{
    
    gBootArgs = (boot_args*)kernel_args;
    gEntryPoint = entryp;
    
    gDeviceTree = (void*)((uint64_t)gBootArgs->deviceTreeP - gBootArgs->virtBase + gBootArgs->physBase);
    
    printf("entryp: %016llx: %08x\n", (uint64_t)entryp, *(uint32_t*)entryp);
    printf("virtBase: %016llx\n", gBootArgs->virtBase);
    printf("physBase: %016llx\n", gBootArgs->physBase);
    
    screen_init();
    
    puts("");
    puts("#==================");
    puts("#");
    printf("# kok3shiOS %s\n", KOKESHI_VERSION);
    puts("#");
    puts("# https://dora2ios.web.app/kokeshi16");
    puts("#");
    puts("#====  Made by  ===");
    puts("# dora2ios");
    puts("#==== Thanks to ===");
    puts("# checkra1n team");
    puts("#==================");
    screen_mark_banner();
    
    printf("Booted by: %s\n", (const char*)dt_get_prop("chosen", "firmware-version", NULL));
    //strcpy(dt_get_prop("chosen", "firmware-version", NULL), "kok3shiOS-");
    //strcat(dt_get_prop("chosen", "firmware-version", NULL), KOKESHI_VERSION);
    
#ifdef __clang__
    printf("Built with: Clang %s\n", __clang_version__);
#else
    printf("Built with: GCC %s\n", __VERSION__);
#endif
    
    gIOBase = dt_get_u64_prop_i("arm-io", "ranges", 1);
    gWDTBase = gIOBase + dt_get_u64_prop("wdt", "reg");
    WDT_CHIP_CTL = 0x0; // Disable WDT
    WDT_SYS_CTL  = 0x0; // Disable WDT
    
//    // alloc_init
//    int tt_bits = 11; // 16k
//    uint64_t pgsz = 1ULL << (tt_bits + 3);
//    ttb_alloc_base = (gBootArgs->physBase + gBootArgs->memSize) & ~(pgsz-1);
//    printf("ttb_alloc_base: %016llx\n", ttb_alloc_base);
//
//    uint64_t memory_size = gBootArgs->memSize;
//    ppages = memory_size >> 14;
//    uint64_t early_heap = ttb_alloc_base;
//
//    early_heap = (early_heap - 4 * ppages) & ~0x3fffULL;
//    ppage_list = (uint32_t*)early_heap;
//    for (uint64_t i = 0; i < ppages; i++) {
//        wired_pages++;
//        ppage_list[i] = PAGE_WIRED; // wire all pages, carve out later.
//    }
//    uint64_t alloc_heap_base = ((gBootArgs->topOfKernelData) + 0x3fffULL) & ~0x3fffULL;
//    uint64_t alloc_heap_end = early_heap;
//    phys_force_free(vatophys_static((void*)alloc_heap_base), alloc_heap_end - alloc_heap_base);
//
//    // alloc static
//    size_t size = (OVERLAY_SIZE + 0x3fffULL) & ~0x3fffULL;
//    disable_interrupts();
//    uint64_t base = (gBootArgs->topOfKernelData + 0x3fffULL) & ~0x3fffULL;
//    uint32_t idx = (base - gBootArgs->physBase) >> 14;
//    for (uint32_t i = 0; i < (size >> 14); ++i) {
//        if (ppage_list[idx + i] != PAGE_FREE) {
//            panic("alloc_static: ran out of static region");
//        }
//        ppage_list[idx + i] = PAGE_WIRED;
//        wired_pages++;
//    }
//    gBootArgs->topOfKernelData = base + size;
//    overlay_base_address = base;
//    enable_interrupts();
//
//    memcpy((void*)overlay_base_address, (void*)OVERLAY_DATA, OVERLAY_SIZE);
    
    kpf();
    
    printf("old bootArgs: %s\n", gBootArgs->CommandLine);
    memcpy((void*)gBootArgs->CommandLine, "rootdev=md0 serial=3 wdt=-1\x00", sizeof("rootdev=md0 serial=3 wdt=-1\x00"));
    printf("new bootArgs: %s\n", gBootArgs->CommandLine);
    
}

extern uint64_t CHIP;
int jump_hook(void* boot_image, void* boot_args)
{
    uint16_t cpid = (uint16_t)CHIP;
    if(iboot_init(cpid))
        return -1;
    
    if (*(uint8_t*)(boot_args + 8 + 7)) {
        // kernel
        payload_entry((uint64_t*)boot_args, boot_image);
    } else {
        // hypv
        payload_entry(*(uint64_t**)(boot_args + 0x20), (void*)*(uint64_t*)(boot_args + 0x28));
    }
    
    puts("Booting...");
    
    return jumpto(boot_image, boot_args);
}

int main(void)
{
    return 0;
}
