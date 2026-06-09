#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/time.h>

#define REPEAT 20

extern uint64_t rdtsc();
extern void clflush(void* p);
extern uint64_t cache_probe(void* p);
extern void fence();
extern void stl(void* store_addr, void* load_addr, uint64_t st_val);


void bindcore(int core) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
}

#define M4P_BLK_THRESHOLD 112
int state_analysis(uint64_t* samples, int samples_len) {
    int state_block = 0;
    // for(int i = 0; i < samples_len; ++ i) {
    //     printf("%llu ", samples[i]);
    // }
    // printf("\n");
    for(int i = 0; i < samples_len; ++ i) {
        if (samples[i] > M4P_BLK_THRESHOLD) {
            state_block += 1;
        }
    }
    return state_block;
}


int main() {
    bindcore(0);
    int A[16];
    for(int i = 0; i < 1000000; ++ i) {
        stl(&A[0], &A[15], 0);
    }

    // Experiment
    char *ptr = malloc(256);
    uint64_t timing_samples[20];
    struct timeval tv1, tv2;
    FILE *file = fopen("./count.txt","w");
    uint64_t results[100005];

    //10s
    for(int j=0; j<12000;j++) {
        gettimeofday(&tv1, NULL);
        gettimeofday(&tv2, NULL);
        int context_switch_time = 0;
        while((tv2.tv_sec * 1000 + tv2.tv_usec / 1000) - (tv1.tv_sec * 1000 + tv1.tv_usec / 1000) < 1) {
            gettimeofday(&tv2, NULL);
            for(int i = 0; i < REPEAT / 2; ++ i) {
                stl(&ptr[20], &ptr[20], 0);
                asm volatile("mfence");
                asm volatile("lfence");
            }

            for(int i = REPEAT / 2; i < REPEAT; ++ i) {
                // predictive STL
                uint64_t t1 = rdtsc();
                stl(&ptr[20], &ptr[20], 0);
                uint64_t t2 = rdtsc();
                timing_samples[i] = t2 - t1;
            }
            int blk_num = state_analysis(&timing_samples[REPEAT / 2], REPEAT / 2);
            if (blk_num >= 7) {
                context_switch_time += 1;
            }
        }
        results[j] = context_switch_time;
    }
    
    for(int j=0;j<12000;j++)
    {
    fprintf(file,"%d\n", results[j]);
    }
    fclose(file);
    
    return 0;
}
