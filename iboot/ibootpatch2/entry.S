
.globl _ORIGINAL
.globl _IBOOT_BASE_ADDRESS
.globl _SDRAM_PAGE_ADDRESS
.globl _LOAD_ADDRESS

.text
.align 4
.org    0x400
.globl start
start:

msr daifset, #0xf
b _patch
nop
nop

_MAGIC:
.quad 0xdeadbeefdeadbeef

_ORIGINAL:
.quad 0x6161616161616161

_IBOOT_BASE_ADDRESS:
.quad 0x6262626262626262

_SDRAM_PAGE_ADDRESS:
.quad 0x6363636363636363

_LOAD_ADDRESS:
.quad 0x6464646464646464

nop
nop
nop
nop
