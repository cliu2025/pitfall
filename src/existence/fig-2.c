/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Chang Liu */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#define REPEAT 100

extern uint64_t rdtsc();
extern void stl(void* store_addr, void* load_addr, uint64_t st_val);

uint64_t* shared_mem;
uint64_t cache_threshold;

void sfp_exp() {
    int A[10];
    // Hot
    for(int i = 0; i < 100000; ++ i) {
        stl(&A[0], &A[8], 0);
    }    
    // Experiment
    uint64_t timing[REPEAT];
    for(int i = 0; i < REPEAT / 2; ++ i) {
        uint64_t t1 = rdtsc();
        stl(&A[0], &A[8], 0);
        uint64_t t2 = rdtsc();
        timing[i] = t2 - t1;
    }
    for(int i = 0; i < REPEAT / 2; ++ i) {
        uint64_t t1 = rdtsc();
        stl(&A[0], &A[0], 0);
        uint64_t t2 = rdtsc();
        timing[i + REPEAT / 2] = t2 - t1;
    }
    // Dump results
    for(int i = 0; i < REPEAT; ++ i) {
        printf("%ld ", timing[i]);
    }
    printf("\n");
}


int main() {
    sfp_exp();
    return 0;
}