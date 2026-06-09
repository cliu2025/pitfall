/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Chang Liu */

.global stl_1
stl_1:
    .rep 30 
    imul $1, %rsi, %rsi
    .endr
    jmp st_l
.align ALIGN_SIZE_ST
st_l:
    movq %rdx, 0x0(%rdi)  // note: if we replace %rdx to an immediate value, we cannot observe the timing difference
    jmp ld_l
.align ALIGN_SIZE_LD
ld_l:
    movq 0x0(%rsi), %rax
    .rep 30 
    imul $1, %rax, %rax
    .endr
    ret

.global stl_2
stl_2:
    .rep 30 
    imul $1, %rsi, %rsi
    .endr
    jmp st_2
.align ALIGN_SIZE_ST
st_2:
    movq %rdx, 0x0(%rdi)  // note: if we replace %rdx to an immediate value, we cannot observe the timing difference
    jmp ld_2
.align ALIGN_SIZE_LD
ld_2:
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
