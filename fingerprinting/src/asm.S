.align 16384
.global stl
stl:
    .rep 10 
    imul $1, %rsi, %rsi
    .endr
    movq %rdx, 0x0(%rdi)
    movq 0x0(%rsi), %rax
    .rep 10
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

.global clflush
clflush:
    mfence
    lfence
    clflush (%rdi)
    mfence
    lfence
    ret