/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Chang Liu */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#define REPEAT 100
#define LOOP_TRY 10

extern uint64_t rdtsc();
extern void stl_1(void* store_addr, void* load_addr, uint64_t st_val);
extern void stl_2(void* store_addr, void* load_addr, uint64_t st_val);

uint64_t* shared_mem;
uint64_t cache_threshold;

int sfp_exp() {
    int A[10];
    // Hot
    for(int i = 0; i < 1000000; ++ i) {
        stl_1(&A[0], &A[8], 0);
        stl_2(&A[0], &A[8], 0);
    }    
    // Experiment
    uint64_t timing[REPEAT];
    for(int i = 0; i < REPEAT / 2; ++ i) {
        uint64_t t1 = rdtsc();
        stl_1(&A[0], &A[0], 0);
        uint64_t t2 = rdtsc();
        timing[i] = t2 - t1;
    }
    for(int i = 0; i < REPEAT / 2; ++ i) {
        uint64_t t1 = rdtsc();
        stl_2(&A[0], &A[0], 0);
        uint64_t t2 = rdtsc();
        timing[i + REPEAT / 2] = t2 - t1;
    }

    int colldided_cnt = 0;
    for(int i = REPEAT / 2; i < REPEAT / 2 + 8; ++ i) {
        int cnt = 0;
        for(int j = REPEAT / 2 + 8; j < REPEAT; ++ j) {
            if (timing[i] > timing[j]) {
                cnt ++;
            }
        }
        if (cnt >= REPEAT / 2 - 10) {
            colldided_cnt ++;
        }
    }
    return colldided_cnt <= 3;
}


int main() {

    int collided_cnt = 0;
    for(int i = 0; i < LOOP_TRY; ++ i) {
        collided_cnt += sfp_exp();
    }

    printf("%d\n", collided_cnt);

    return 0;
}