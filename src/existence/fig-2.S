/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Chang Liu */

.global stl
stl:
    .rep 30 
    imul $1, %rsi, %rsi
    .endr
    movq %rdx, 0x0(%rdi)  // note: if we replace %rdx to an immediate value, we cannot observe the timing difference
    movq 0x0(%rsi), %rax
    .rep 30 
    imul $1, %rax, %rax
    .endr
    ret

.global rdtsc
rdtsc:
    mfence
    lfence
    rdtscp
    shl $32, %rdx
    or %rdx, %rax
    ret
