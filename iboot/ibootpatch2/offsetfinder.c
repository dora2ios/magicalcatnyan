/*
 * patchfinder64.c
 *
 * Created by Vladimir Putin on 14.12.16.
 * Fixes by Alex Hude and Max Bazaliy
 * Updated by sakuRdev on 2021/12/02.
 * Some parts of code from Luca Todesco and Pangu
 *
 * Copyright (c) 2016 -2017 FriedApple Team.
 * Copyright (c) 2021 sakuRdev.
 * Copyright (c) 2022 dora2ios.
 *
 * This file is part of kokeshidoll.
 *
 */

#include <stdint.h>
#include <common.h>
#include <offsetfinder.h>

static uint32_t* find_insn_maskmatch_match(uint8_t* data, size_t size, uint32_t* matches, uint32_t* masks, int count)
{
    int found = 0;
    if(sizeof(matches) != sizeof(masks))
        return NULL;
    
    uint32_t* current_inst = (uint32_t*)data;
    while((uintptr_t)current_inst < (uintptr_t)data + size - 4 - (count*4)) {
        current_inst++;
        found = 1;
        for(int i = 0; i < count; i++)
        {
            if((matches[i] & masks[i]) != (current_inst[i] & masks[i]))
            {
                found = 0;
                break;
            }
        }
        if(found)
        {
            return current_inst;
        }
    }
    
    return NULL;
}

static uint32_t* find_next_insn_matching_64(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* current_instruction, int (*match_func)(uint32_t*))
{
    while((uintptr_t)current_instruction < (uintptr_t)kdata + ksize - 4) {
        current_instruction++;
        
        if(match_func(current_instruction)) {
            return current_instruction;
        }
    }
    
    return NULL;
}

static uint32_t* find_insn_matching_64_with_count(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* current_instruction, int (*match_func)(uint32_t*), unsigned int count)
{
    unsigned int cnt = 0;
    while((uintptr_t)current_instruction < (uintptr_t)kdata + ksize - 4) {
        current_instruction++;
        
        if(match_func(current_instruction)) {
            if(cnt == count)
            {
                return current_instruction;
            }
            cnt++;
        }
        
    }
    
    return NULL;
}

static uint32_t bit_range_64(uint32_t x, int start, int end)
{
    x = (x << (31 - start)) >> (31 - start);
    x = (x >> end);
    return x;
}

static uint64_t real_signextend_64(uint64_t imm, uint8_t bit)
{
    if ((imm >> bit) & 1) {
        return (-1LL << (bit + 1)) + imm;
    } else
        return imm;
}

static uint64_t signextend_64(uint64_t imm, uint8_t bit)
{
    //assert(bit > 0);
    return real_signextend_64(imm, bit - 1);
    /*
     if ((imm >> bit) & 1)
     return (uint64_t)(-1) - (~((uint64_t)1 << bit)) + imm;
     else
     return imm;
     */
}

static int insn_is_mov_imm_64(uint32_t* i)
{
    if ((*i & 0x7f800000) == 0x52800000)
        return 1;
    
    return 0;
}

static int insn_is_mov_reg64(uint32_t* i)
{
    return (*i & 0x7FE003E0) == 0x2A0003E0;
}

static int insn_mov_reg_rd64(uint32_t* i)
{
    return *i & 0x1F;
}

static int insn_mov_imm_rd_64(uint32_t* i)
{
    return (*i & 0x1f);
}

static uint32_t insn_mov_imm_imm_64(uint32_t* i)
{
    return bit_range_64(*i, 20, 5);
}

static int insn_is_ldr_literal_64(uint32_t* i)
{
    // C6.2.84 LDR (literal) LDR Xt
    if ((*i & 0xff000000) == 0x58000000)
        return 1;
    
    // C6.2.84 LDR (literal) LDR Wt
    if ((*i & 0xff000000) == 0x18000000)
        return 1;
    
    // C6.2.95 LDR (literal) LDRSW Xt
    if ((*i & 0xff000000) == 0x98000000)
        return 1;
    
    return 0;
}

static int insn_is_ret(uint32_t* i)
{
    if (*i == 0xd65f03c0)
        return 1;
    
    return 0;
}

static int insn_add_reg_rm_64(uint32_t* i)
{
    return ((*i >> 16) & 0x1f);
}

static int insn_ldr_literal_rt_64(uint32_t* i)
{
    return (*i & 0x1f);
}

static uint64_t insn_ldr_literal_imm_64(uint32_t* i)
{
    uint64_t imm = (*i & 0xffffe0) >> 3;
    return signextend_64(imm, 21);
}

static uint64_t insn_adr_imm_64(uint32_t* i)
{
    uint64_t immhi = bit_range_64(*i, 23, 5);
    uint64_t immlo = bit_range_64(*i, 30, 29);
    uint64_t imm = (immhi << 2) + (immlo);
    return signextend_64(imm, 19+2);
}

static uint64_t insn_adrp_imm_64(uint32_t* i)
{
    uint64_t immhi = bit_range_64(*i, 23, 5);
    uint64_t immlo = bit_range_64(*i, 30, 29);
    uint64_t imm = (immhi << 14) + (immlo << 12);
    return signextend_64(imm, 19+2+12);
}

static int insn_is_adrp_64(uint32_t* i)
{
    if ((*i & 0x9f000000) == 0x90000000) {
        return 1;
    }
    
    return 0;
}

static int insn_adrp_rd_64(uint32_t* i)
{
    return (*i & 0x1f);
}

static int insn_is_add_imm_64(uint32_t* i)
{
    if ((*i & 0x7f000000) == 0x11000000)
        return 1;
    
    return 0;
}

static int insn_add_imm_rd_64(uint32_t* i)
{
    return (*i & 0x1f);
}

static int insn_add_imm_rn_64(uint32_t* i)
{
    return ((*i >> 5) & 0x1f);
}

static uint64_t insn_add_imm_imm_64(uint32_t* i)
{
    uint64_t imm = bit_range_64(*i, 21, 10);
    if (((*i >> 22) & 3) == 1)
        imm = imm << 12;
    return imm;
}

static int insn_add_reg_rd_64(uint32_t* i)
{
    return (*i & 0x1f);
}

static int insn_add_reg_rn_64(uint32_t* i)
{
    return ((*i >> 5) & 0x1f);
}

static int insn_is_add_reg_64(uint32_t* i)
{
    if ((*i & 0x7fe00c00) == 0x0b200000)
        return 1;
    
    return 0;
}

static int insn_is_adr_64(uint32_t* i)
{
    if ((*i & 0x9f000000) == 0x10000000)
        return 1;
    
    return 0;
}

static int insn_is_bl_64(uint32_t* i)
{
    if ((*i & 0xfc000000) == 0x94000000)
        return 1;
    else
        return 0;
}

static uint64_t insn_bl_imm32_64(uint32_t* i)
{
    uint64_t imm = (*i & 0x3ffffff) << 2;
    //PFExtLog("imm = %llx\n", imm);
    // sign extend
    uint64_t res = real_signextend_64(imm, 27);
    
    //PFExtLog("real_signextend_64 = %llx\n", res);
    
    return res;
}

int insn_is_funcbegin_64(uint32_t* i)
{
    if (*i == 0xa9bf7bfd)
        return 1;
    if (*i == 0xa9bc5ff8)
        return 1;
    if (*i == 0xa9bd57f6)
        return 1;
    if (*i == 0xa9ba6ffc)
        return 1;
    if (*i == 0xa9bb67fa)
        return 1;
    if (*i == 0xa9be4ff4)
        return 1;
    return 0;
}


static uint32_t* find_literal_ref_64(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* insn, uint64_t address)
{
    uint32_t* current_instruction = insn;
    uint64_t registers[32];
    memset(registers, 0, sizeof(registers));
    
    while((uintptr_t)current_instruction < (uintptr_t)(kdata + ksize))
    {
        if (insn_is_mov_imm_64(current_instruction))
        {
            int reg = insn_mov_imm_rd_64(current_instruction);
            uint64_t value = insn_mov_imm_imm_64(current_instruction);
            registers[reg] = value;
            //PFExtLog("%s:%d mov (immediate): reg[%d] is reset to %#llx\n", __func__, __LINE__, reg, value);
        }
        else if (insn_is_ldr_literal_64(current_instruction))
        {
            uintptr_t literal_address  = (uintptr_t)current_instruction + (uintptr_t)insn_ldr_literal_imm_64(current_instruction);
            if(literal_address >= (uintptr_t)kdata && (literal_address + 4) <= ((uintptr_t)kdata + ksize))
            {
                int reg = insn_ldr_literal_rt_64(current_instruction);
                uint64_t value =  *(uint64_t*)(literal_address);
                registers[reg] = value;
                //PFExtLog("%s:%d ldr (literal): reg[%d] is reset to %#llx\n", __func__, __LINE__, reg, value);
            }
        }
        else if (insn_is_adrp_64(current_instruction))
        {
            int reg = insn_adrp_rd_64(current_instruction);
            uint64_t value = ((((uintptr_t)current_instruction - (uintptr_t)kdata) >> 12) << 12) + insn_adrp_imm_64(current_instruction);
            registers[reg] = value;
            //PFExtLog("%s:%d adrp: reg[%d] is reset to %#llx\n", __func__, __LINE__, reg, value);
        }
        else if (insn_is_adr_64(current_instruction))
        {
            uint64_t value = (uintptr_t)current_instruction - (uintptr_t)kdata + insn_adr_imm_64(current_instruction);
            if (value == address)
            {
                //PFExtLog("%s:%d FINAL pointer is %#llx\n", __func__, __LINE__, (uint64_t)current_instruction - (uint64_t)kdata);
                return current_instruction;
            }
        }
        else if(insn_is_add_reg_64(current_instruction))
        {
            int reg = insn_add_reg_rd_64(current_instruction);
            if(insn_add_reg_rm_64(current_instruction) == 15 && insn_add_reg_rn_64(current_instruction) == reg)
            {
                uint64_t value = ((uintptr_t)current_instruction - (uintptr_t)kdata) + 4;
                registers[reg] += value;
                //PFExtLog("%s:%d adrp: reg[%d] += %#llx\n", __func__, __LINE__, reg, value);
                if(registers[reg] == address)
                {
                    //PFExtLog("%s:%d FINAL pointer is %#llx\n", __func__, __LINE__, (uint64_t)current_instruction - (uint64_t)kdata);
                    return current_instruction;
                }
            }
        }
        else if (insn_is_add_imm_64(current_instruction))
        {
            int reg = insn_add_imm_rd_64(current_instruction);
            if (insn_add_imm_rn_64(current_instruction) == reg)
            {
                uint64_t value = insn_add_imm_imm_64(current_instruction);
                registers[reg] += value;
                //PFExtLog("%s:%d adrp: reg[%d] += %#llx\n", __func__, __LINE__, reg, value);
                if (registers[reg] == address)
                {
                    //PFExtLog("%s:%d FINAL pointer is %#llx\n", __func__, __LINE__, (uint64_t)current_instruction - (uint64_t)kdata);
                    return current_instruction;
                }
            }
            
        }
        
        current_instruction++;
    }
    
    //PFExtLog("%s:%d FINAL pointer is NULL\n", __func__, __LINE__);
    return NULL;
}

// search next instruction, decrementing mode
static uint32_t* find_last_insn_matching_64(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* current_instruction, int (*match_func)(uint32_t*))
{
    while((uintptr_t)current_instruction > (uintptr_t)kdata) {
        current_instruction--;
        
        if(match_func(current_instruction)) {
            return current_instruction;
        }
    }
    
    return NULL;
}






uint64_t find_check_bootmode(uint64_t region, uint8_t* data, size_t size)
{
    uint8_t* str = memmem(data, size, "debug-uarts", sizeof("debug-uarts"));
    if(!str)
        return 0;
    
    // Find a reference to the string.
    uint32_t* ref = find_literal_ref_64(region, data, size, (uint32_t*)data, (uintptr_t)str - (uintptr_t)data);
    if (!ref)
        return 0;
    
    // find 3rd-BL
    uint32_t *bl_addr = find_insn_matching_64_with_count(region, data, size, ref, insn_is_bl_64, 2);
    if (!bl_addr)
        return 0;
    
    return ((uintptr_t)bl_addr - (uintptr_t)data) + insn_bl_imm32_64(bl_addr);
}


uint64_t find_sigcheck(uint64_t region, uint8_t* data, size_t size)
{
    uint32_t* ptr = memmem(data, size, "\xC1\xAE\x8C\x52\x61\xEE\xAD\x72", 8);
    if(!ptr)
        return 0;
    
    // find RET
    uint32_t *ret_addr = find_next_insn_matching_64(region, data, size, ptr, insn_is_ret);
    if (!ret_addr)
        return 0;
    
    // find mov X0, Xy
    uint32_t *mov_addr = NULL;
    
    while(1)
    {
        mov_addr = find_last_insn_matching_64(region, data, size, ret_addr, insn_is_mov_reg64);
        if (!mov_addr)
            return 0;
        if(insn_mov_reg_rd64(mov_addr) == 0)
            break;
        mov_addr--;
    }
    
    return ((uintptr_t)mov_addr - (uintptr_t)data);
}


static uint64_t find_memcmp(uint64_t region, uint8_t* data, size_t size)
{
    
    uint32_t matches_variant[] = {
        0xb4000122,
        0x39400008,
        0x39400029,
        0x6b090108,
    };
    uint32_t masks_variant[] = {
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
    };
    
    uint32_t* ref = find_insn_maskmatch_match(data, size, matches_variant, masks_variant, sizeof(matches_variant)/sizeof(uint32_t));
    
    if(!ref)
    {
        uint32_t matches[] = {
            0x52800008,
            0xb4000142,
            0x39400008,
            0x39400029,
            0x4b090108,
        };
        uint32_t masks[] = {
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
        };
        
        ref = find_insn_maskmatch_match(data, size, matches, masks, sizeof(matches)/sizeof(uint32_t));
        
        if(!ref)
            return 0;
        
    }
        
    
    return (uintptr_t)ref - (uintptr_t)data;
}

uint64_t find_boot_manifest_validation(uint64_t region, uint8_t* data, size_t size)
{
    
    uint64_t memcmp_off = find_memcmp(region, data, size);
    if(!memcmp_off)
        return 0;
    
    uint32_t* ref = NULL;
    
    uint32_t matches_variant[] = {
        0x2A0003E0, // mov x0, xn
        0x2A0003E1, // mov x1, xn
        0x52800602, // mov w2, #0x30
        0x94000000, // BL _x
    };
    uint32_t masks_variant[] = {
        0x7FE003FF,
        0x7FE003FF,
        0xffffffff,
        0xFC000000,
    };
    
    uint8_t* start = data;
    while(1)
    {
        ref = find_insn_maskmatch_match(start, size, matches_variant, masks_variant, sizeof(matches_variant)/sizeof(uint32_t));
        if(!ref)
            return 0;
        ref += 3; // BL _x
        
        if(((uintptr_t)ref - (uintptr_t)data + insn_bl_imm32_64(ref)) == memcmp_off)
            return (uintptr_t)ref - (uintptr_t)data;
        if(size < ((uintptr_t)ref - (uintptr_t)data))
            return 0;
        size = size - ((uintptr_t)ref - (uintptr_t)data);
        if(size < 0)
            return 0;
        start = (uint8_t*)ref;
        
    }
    
}

uint64_t find_zero(uint64_t region, uint8_t* data, size_t size)
{
    unsigned char zeroBuf[0x200];
    memset(&zeroBuf, '\0', 0x200);
    uint32_t* zero = memmem(data, size, zeroBuf, 0x200);
    if(!zero)
        return 0;
    
    zero += 6;
    
    uint64_t address = ((uintptr_t)zero - (uintptr_t)data);
    address = address &~ 0x7;
    
    return address;
}

uint64_t find_go_cmd_handler(uint64_t region, uint8_t* data, size_t size)
{
    if(!region)
        return 0;
    
    uint64_t maxsize = region + size;
    uint64_t search[1];
    
    uint8_t* str = memmem(data, size, "\x00go\x00", 4);
    if(!str)
        return 0;
    str += 1;
    
    uint64_t addr = ((uintptr_t)str - (uintptr_t)data) + region;
    if(maxsize < addr)
        return 0;
    
    search[0] = addr;
    uint8_t* ptr = memmem(data, size, search, sizeof(search));
    if(!ptr)
        return 0;
    
    return ((uintptr_t)ptr - (uintptr_t)data) + 8;
}

uint64_t find_bootx_cmd_handler(uint64_t region, uint8_t* data, size_t size)
{
    if(!region)
        return 0;
    
    uint64_t maxsize = region + size;
    uint64_t search[1];
    
    uint8_t* str = memmem(data, size, "\x00\x62\x6F\x6F\x74\x78\x00", 7);
    if(!str)
        return 0;
    str += 1;
    
    uint64_t addr = ((uintptr_t)str - (uintptr_t)data) + region;
    if(maxsize < addr)
        return 0;
    
    search[0] = addr;
    uint8_t* ptr = memmem(data, size, search, sizeof(search));
    if(!ptr)
        return 0;
    
    return ((uintptr_t)ptr - (uintptr_t)data) + 8;
}


uint64_t find_reset_cmd_handler(uint64_t region, uint8_t* data, size_t size)
{
    if(!region)
        return 0;
    
    uint64_t maxsize = region + size;
    uint64_t search[1];
    
    uint8_t* str = memmem(data, size, "\x00\x72\x65\x73\x65\x74\x00", 7);
    if(!str)
        return 0;
    str += 1;
    
    uint64_t addr = ((uintptr_t)str - (uintptr_t)data) + region;
    if(maxsize < addr)
        return 0;
    
    search[0] = addr;
    uint8_t* ptr = memmem(data, size, search, sizeof(search));
    if(!ptr)
        return 0;
    
    return ((uintptr_t)ptr - (uintptr_t)data) + 8;
}



uint64_t find_mount_and_boot_system(uint64_t region, uint8_t* data, size_t size)
{
    if(!region)
        return 0;
    
    uint64_t maxsize = region + size;
    uint64_t search[1];
    
    uint8_t* str = memmem(data, size, "\x00\x66\x73\x62\x6F\x6F\x74\x00", 8);
    if(!str)
        return 0;
    str += 1;
    
    uint64_t addr = ((uintptr_t)str - (uintptr_t)data) + region;
    if(maxsize < addr)
        return 0;
    
    search[0] = addr;
    uint8_t* ptr = memmem(data, size, search, sizeof(search));
    if(!ptr)
        return 0;
    
    uint64_t offset = ((uintptr_t)ptr - (uintptr_t)data) + 8;
    if(size < offset)
        return 0;
    
    uint64_t* dq = (uint64_t*)(data + offset);
    if(!dq)
        return 0;
    
    if(dq[0] < region)
        return 0;
    
    uint64_t ret = dq[0] - region;
    
    return ret;
}


uint64_t find_jumpto_bl(uint64_t region, uint8_t* data, size_t size)
{
    uint32_t search[2];
    
    uint8_t* str = memmem(data, size, "======== End of %s serial output. ========\n", sizeof("======== End of %s serial output. ========\n"));
    if(!str)
        return 0;
    
    // Find a reference to the string.
    uint32_t* ref = find_literal_ref_64(region, data, size, (uint32_t*)data, (uintptr_t)str - (uintptr_t)data);
    if (!ref)
        return 0;
    
    search[0] = 0xaa1403e0; // mov x0, x20
    search[1] = 0xaa1303e1; // mov x1, x19
    
    uint32_t* jump = memmem(ref, size - ((uintptr_t)ref - (uintptr_t)data), search, sizeof(search));
    if (!jump)
        return 0;
    
    uint32_t *bl_addr = find_next_insn_matching_64(region, data, size, jump, insn_is_bl_64);
    if (!bl_addr)
        return 0;
    
    return ((uintptr_t)bl_addr - (uintptr_t)data);
}

uint64_t find_ptr_obfuscation(uint64_t region, uint8_t* data, size_t size)
{
    uint8_t* str = memmem(data, size, "<ptr>", sizeof("<ptr>"));
    if(!str)
        return 0;
    
    uint32_t* ref = find_literal_ref_64(region, data, size, (uint32_t*)data, (uintptr_t)str - (uintptr_t)data);
    if (!ref)
        return 0;
    
    return ((uintptr_t)ref - (uintptr_t)data);
}

