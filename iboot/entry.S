/*
 * pongoOS - https://checkra.in
 *
 * Copyright (C) 2019-2021 checkra1n team
 *
 * This file is part of pongoOS.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

.globl _MEMCPY_OFFSET
.globl _TRAMPOLINE

.globl _HEAP_BASE
.globl _HEAP_WRITE_OFFSET
.globl _HEAP_STATE
.globl _HEAP_WRITE_HASH
.globl _HEAP_CHECK_ALL
.globl _DFU_BOOL
.globl _BOOTSTRAP_TASK_LR
.globl _NAND_BOOT_JUMP

.globl start
.align 4
start:

// 0x0000
b start

.org 0x800
b _payload
nop
nop
nop

_MEMCPY_OFFSET:
.quad 0x5151515151515151

_TRAMPOLINE:
.quad 0x5252525252525252

_HEAP_BASE:
.quad 0x5353535353535353

_HEAP_WRITE_OFFSET:
.quad 0x5454545454545454

_HEAP_STATE:
.quad 0x5555555555555555

_HEAP_WRITE_HASH:
.quad 0x5656565656565656

_HEAP_CHECK_ALL:
.quad 0x5757575757575757

_DFU_BOOL:
.quad 0x5858585858585858

_BOOTSTRAP_TASK_LR:
.quad 0x5959595959595959

_NAND_BOOT_JUMP:
.quad 0x5a5a5a5a5a5a5a5a

nop
nop
nop
nop
