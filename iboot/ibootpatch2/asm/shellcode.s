.text

.pool

.set RELOCATED,         0xdeafbeefdeaf0001 // 0x800E00000
.set LOAD_ADDRESS,      0xdeafbeefdeaf0002 // 0x801000000
.set SDRAM_PAGE,        0xdeafbeefdeaf0003 // 0x180002000
.set OVERLAY,           0xdeafbeefdeaf0004 // 0x800F00000

.set ARM_TTE_BLOCK_PNX, 0x0020000000000000
.set ARM_TTE_BLOCK_NX,  0x0040000000000000

.global _main
_main:
B       _real_main
B       _tram
B       _relocate_overlay
NOP

_real_main:
LDR     X1,  =RELOCATED
LDR     X0,  =LOAD_ADDRESS
MOV     X2,  #0
MOV     X3,  #0x80000

loop:
LDP     X4, X5,  [X0]
STP     X4, X5,  [X1]
ADD     X0, X0, #0x10
ADD     X1, X1, #0x10
ADD     X2, X2, #0x10
CMP     X2, X3
B.CC    loop
B       _dorwx
RET

_dorwx:
MOV     X5, X30
LDR     X0, =0x800000000
BL      _cache_clean_and_invalidate_page
MOV     X0, #0
SVC     #0
IC      IALLU

MRS     X4, SCTLR_EL1
MOV     X0, #0
MSR     SCTLR_EL1, x0

LDR     X0, =SDRAM_PAGE
LDR     X0, [X0]
BIC     X0, X0, (ARM_TTE_BLOCK_PNX | ARM_TTE_BLOCK_NX)
LDR     X1, =SDRAM_PAGE
STR     X0, [X1]

MOV     X0, X4
BIC     X0, X0, #0x80000
MSR     SCTLR_EL1, X0

DSB     SY
TLBI    VMALLE1
DSB     SY
ISB

MRS     X0, SPSR_EL1
AND     X0, X0, #0xFFFFFFFFFFFFFFF3
MSR     SPSR_EL1, X0
MOV     X0, X5
MSR     ELR_EL1, X0
ERET

_cache_clean_and_invalidate_page:
MOV     X1, #0x80000
MOV     X2, #0

_one:
CMP     X1, X2
B.EQ    _two
DC      CIVAC, X0
ADD     X0, X0, #0x40
ADD     X2, X2, #0x40
B       _one

_two:
RET

_relocate_overlay:
LDR     X1,  =OVERLAY
LDR     X0,  =LOAD_ADDRESS
MOV     X2,  #0
MOV     X3,  #0x80000
copy_loop:
LDP     X4, X5,  [X0]
STP     X4, X5,  [X1]
ADD     X0, X0, #0x10
ADD     X1, X1, #0x10
ADD     X2, X2, #0x10
CMP     X2, X3
B.CC    copy_loop
MOV     X0,  #0
RET

_tram:
LDR     X16, =RELOCATED
ADD     X16, X16, #4
BR      X16
