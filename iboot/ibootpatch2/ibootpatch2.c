#include <stdint.h>
#include <stdbool.h>

#include <common.h>
#include <offsetfinder.h>

#include "asm/shellcode.h"

#define INSN_MOV_X0_0           0xd2800000
#define INSN_MOV_X0_1           0xd2800020
#define INSN_RET                0xd65f03c0
#define INSN_NOP                0xd503201f

#ifndef TESTBUILD
#define LOG(x, ...)
#define ERR(x, ...)
#define DEVLOG(x, ...)
#else
#define LOG(x, ...) \
do { \
printf("[LOG] "x"\n", ##__VA_ARGS__); \
} while(0)

#define ERR(x, ...) \
do { \
printf("[ERR] "x"\n", ##__VA_ARGS__); \
} while(0)
#ifdef DEVBUILD
#define DEVLOG(x, ...) \
do { \
printf("[DEV] "x"\n", ##__VA_ARGS__); \
} while(0)
#else
#define DEVLOG(x, ...)
#endif
#endif

#define sub     (0x000200000)
#define overlay (0x000100000)
typedef int (*jump_t)(void* arg0, void* arg1);

void patch_iboot(uint64_t iboot_base, void* idata, size_t isize,
                 uint64_t sdram_page, uint64_t load_address)

{
    // find iboot
    LOG("---- offsetfinder: start ----");
    
    
    uint64_t sigcheck = find_sigcheck(iboot_base, idata, isize);
    if(!sigcheck) {
        ERR("Failed to find sigcheck");
        goto end;
    }
    DEVLOG("%016llx[%016llx]: sigcheck", sigcheck + iboot_base, sigcheck);
    
    
    uint64_t boot_manifest_validation = find_boot_manifest_validation(iboot_base, idata, isize);
    if(!boot_manifest_validation) {
        ERR("Failed to find boot_manifest_validation");
        goto end;
    }
    DEVLOG("%016llx[%016llx]: boot_manifest_validation", boot_manifest_validation + iboot_base, boot_manifest_validation);
    
    
    uint64_t check_bootmode = find_check_bootmode(iboot_base, idata, isize);
    if(!check_bootmode) {
        ERR("Failed to find check_bootmode");
        goto end;
    }
    DEVLOG("%016llx[%016llx]: check_bootmode", check_bootmode + iboot_base, check_bootmode);
    
    
    uint64_t zero_region = find_zero(iboot_base, idata, isize);
    if(!zero_region) {
        ERR("Failed to find zero_region");
        goto end;
    }
    DEVLOG("%016llx[%016llx]: zero_region", zero_region + iboot_base, zero_region);
    
    
    uint64_t go_cmd_handler = find_go_cmd_handler(iboot_base, idata, isize);
    if(!go_cmd_handler) {
        ERR("Failed to find go_cmd_handler");
        goto end;
    }
    DEVLOG("%016llx[%016llx]: go_cmd_handler", go_cmd_handler + iboot_base, go_cmd_handler);
    
    
    uint64_t bootx_cmd_handler = find_bootx_cmd_handler(iboot_base, idata, isize);
    if(!bootx_cmd_handler) {
        ERR("Failed to find bootx_cmd_handler");
        goto end;
    }
    DEVLOG("%016llx[%016llx]: bootx_cmd_handler", bootx_cmd_handler + iboot_base, bootx_cmd_handler);
    
    
    uint64_t reset_cmd_handler = find_reset_cmd_handler(iboot_base, idata, isize);
    if(!reset_cmd_handler) {
        ERR("Failed to find reset_cmd_handler");
        goto end;
    }
    DEVLOG("%016llx[%016llx]: reset_cmd_handler", reset_cmd_handler + iboot_base, reset_cmd_handler);
    
    
    uint64_t _mount_and_boot_system = find_mount_and_boot_system(iboot_base, idata, isize);
    if(!_mount_and_boot_system) {
        ERR("Failed to find _mount_and_boot_system");
        goto end;
    }
    DEVLOG("%016llx[%016llx]: _mount_and_boot_system", _mount_and_boot_system + iboot_base, _mount_and_boot_system);
    
    
    uint64_t jumpto_bl = find_jumpto_bl(iboot_base, idata, isize);
    if(!jumpto_bl)
        goto end;
    DEVLOG("%016llx[%016llx]: jumpto_bl", jumpto_bl + iboot_base, jumpto_bl);
    
//
//    uint64_t ptr_obfuscation = find_ptr_obfuscation(iboot_base, idata, isize);
//    if(!ptr_obfuscation)
//        goto end;
//    DEVLOG("%016llx[%016llx]: ptr_obfuscation", ptr_obfuscation + iboot_base, ptr_obfuscation);
    
    LOG("---- offsetfinder: done ----");
    
    /*---- patch part ----*/
    
    /*
     REMOTE_BOOT
     $ ./irecovery -s
     > saveenv
     > /upload ramdisk.img4
     > ramdisk
     > /upload payload.bin
     > go
     > bootx
     
     */
    
    LOG("---- patch: start ----");
    {
        {
            uint32_t* patch_check_bootmode = (uint32_t*)(idata + check_bootmode);
            uint32_t opcode = INSN_MOV_X0_1;  // 0: LOCAL_BOOT, 1: REMOTE_BOOT
            if((opcode & 0xffffffdf) != 0xd2800000)
            {
                ERR("Detected weird opcode");
                goto end;
            }
            patch_check_bootmode[0] = opcode;
            patch_check_bootmode[1] = INSN_RET;
            LOG("bootmode=%d (%s)", ((opcode & 0xf0) >> 5), ((opcode & 0xf0) >> 5) == 0 ? "LOCAL_BOOT" : "REMOTE_BOOT");
        }
        
        // shellcode injection
        {
            // 1, set offsets
            LOG("setting offsets...");
            size_t point = shellcode_bin_len - (sizeof(uint64_t) * 4);
            // 1: RELOCATED
            // 2: LOAD_ADDRESS
            // 3: SDRAM_PAGE
            uint64_t* offset = (uint64_t*)(shellcode_bin + point);
            offset[0] = load_address - sub;
            offset[1] = load_address;
            offset[2] = sdram_page;
            offset[3] = load_address - overlay;
            
        }
        
        {
            // 2, copy payload.bin
            LOG("copying payload...");
            memcpy((void*)(idata + zero_region), shellcode_bin, shellcode_bin_len);
            LOG("done");
        }
        
        {
            // 3, relocate go cmd
            uint64_t cmd_hander = 0;
            
            cmd_hander = go_cmd_handler;
            
            uint64_t* patch_go_cmd_handler = (uint64_t*)(idata + cmd_hander);
            patch_go_cmd_handler[0] = iboot_base + zero_region;
            DEVLOG("new go cmd handler: %016llx", iboot_base + zero_region);
            LOG("relocated go cmd handler");
        }
        
        {
            // 4, relocate bootx cmd (set fsboot)
            uint64_t cmd_hander = 0;
            
            cmd_hander = bootx_cmd_handler;
            
            uint64_t* patch_bootx_cmd_handler = (uint64_t*)(idata + cmd_hander);
            patch_bootx_cmd_handler[0] = _mount_and_boot_system + iboot_base;
            DEVLOG("new bootx cmd handler: %016llx", _mount_and_boot_system + iboot_base);
            LOG("relocated bootx cmd handler");
        }
        
        {
            // 5, relocate reset cmd (set rwx)
            uint64_t* patch_reset_cmd_handler = (uint64_t*)(idata + reset_cmd_handler);
            patch_reset_cmd_handler[0] = iboot_base + zero_region + 8;
            DEVLOG("new reset cmd handler: %016llx", iboot_base + zero_region + 8);
            LOG("relocated reset cmd handler");
        }
        
        
        {
            // 6, hook jumpto xnu
            uint32_t* patch_jumpto = (uint32_t*)(idata + jumpto_bl);
            
            uint64_t new_jumpto_addr = zero_region + 4;
            DEVLOG("%016llx: BL %016llx", jumpto_bl + iboot_base, new_jumpto_addr + iboot_base);
            int64_t delta = new_jumpto_addr - jumpto_bl;
            uint32_t opcode = 0x94000000 | (((uint64_t)delta >> 2) & 0x3ffffff);
            DEVLOG("opcode: %08x", opcode);
            patch_jumpto[0] = opcode;
            LOG("jumpto");
        }
        
    }
    
    {
        uint32_t* patch_sigcheck = (uint32_t*)(idata + sigcheck);
        patch_sigcheck[0] = INSN_MOV_X0_0;
        LOG("signature check");
    }
    
    {
        uint32_t* patch_boot_manifest_validation = (uint32_t*)(idata + boot_manifest_validation);
        patch_boot_manifest_validation[0] = INSN_MOV_X0_0;
        LOG("boot_manifest hash validation");
    }
    
    LOG("---- patch: done ----");
    
end:
    return;
}

#ifndef TESTBUILD
extern uint64_t ORIGINAL;
extern uint64_t IBOOT_BASE_ADDRESS;
extern uint64_t SDRAM_PAGE_ADDRESS;
extern uint64_t LOAD_ADDRESS;

int patch(void* arg0, void* arg1)
{
    
    // t8015, TODO: offsetfinder
    jump_t ret = (jump_t)ORIGINAL;// (jump_t)0x180018004;
    uint64_t iboot_base = IBOOT_BASE_ADDRESS; // 0x18001c000;
    
    void* idata = (void*)(iboot_base);
    uint64_t isize = *(uint64_t*)(iboot_base + 0x308) - iboot_base; // ... why 0x308??
    
    uint64_t sdram_page = SDRAM_PAGE_ADDRESS; // 0x180002000;
    uint64_t loadaddr = LOAD_ADDRESS; // 0x801000000;
    
    patch_iboot(iboot_base, idata, isize, sdram_page, loadaddr);
    
    return ret(arg0, arg1);
}

#endif
