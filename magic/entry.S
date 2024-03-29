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

.globl _CHIP
.globl _OVERLAY_DATA
.globl _OVERLAY_SIZE

.globl _enable_interrupts
.globl _disable_interrupts

.globl start
.align 4
start:

// 0x0000
b _payload
b _jump_hook
nop
nop

_CHIP:
.quad 0x7171717171717171

_OVERLAY_DATA:
.quad 0x7272727272727272

_OVERLAY_SIZE:
.quad 0x7373737373737373

nop
nop
nop
nop

_enable_interrupts:
msr daifclr,#0xf
isb
ret
_disable_interrupts:
msr daifset,#0xf
isb
ret
