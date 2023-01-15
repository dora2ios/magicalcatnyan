#ifndef OFFSETFINDER_H
#define OFFSETFINDER_H

#include <stdint.h>
#include <common.h>

uint64_t find_check_bootmode(uint64_t region, uint8_t* data, size_t size);
uint64_t find_sigcheck(uint64_t region, uint8_t* data, size_t size);
uint64_t find_boot_manifest_validation(uint64_t region, uint8_t* data, size_t size);
uint64_t find_zero(uint64_t region, uint8_t* data, size_t size);
uint64_t find_go_cmd_handler(uint64_t region, uint8_t* data, size_t size);
uint64_t find_jumpto_bl(uint64_t region, uint8_t* data, size_t size);
uint64_t find_bootx_cmd_handler(uint64_t region, uint8_t* data, size_t size);
uint64_t find_mount_and_boot_system(uint64_t region, uint8_t* data, size_t size);
uint64_t find_reset_cmd_handler(uint64_t region, uint8_t* data, size_t size);
uint64_t find_ptr_obfuscation(uint64_t region, uint8_t* data, size_t size);

#endif
