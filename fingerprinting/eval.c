#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>

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

int sfp_exp(int context_switch) {
    // Experiment
    char *ptr = malloc(256);
    uint64_t timing_samples[20];

    for(int i = 0; i < REPEAT / 2; ++ i) {
        // predictive STL
        uint64_t t1 = rdtsc();
        stl(&ptr[20], &ptr[20], 0);
        uint64_t t2 = rdtsc();
        timing_samples[i] = t2 - t1;
    }

    if(context_switch) {
        usleep(1);
    }

    for(int i = REPEAT / 2; i < REPEAT; ++ i) {
        // predictive STL
        uint64_t t1 = rdtsc();
        stl(&ptr[20], &ptr[20], 0);
        uint64_t t2 = rdtsc();
        timing_samples[i] = t2 - t1;
    }
    int blk_num = state_analysis(&timing_samples[REPEAT / 2], REPEAT / 2);
    return blk_num;
}

int main() {
    bindcore(0);
    int A[16];
    for(int i = 0; i < 1000000; ++ i) {
        stl(&A[0], &A[15], 0);
    }
    int tp = 0, tn = 0, fp = 0, fn = 0;
    for(int i = 0; i < 100000; ++ i) {
        int delay = rand() % 2;
        int res = sfp_exp(delay);
        if (delay == 1 && res >= 7) {
            tp += 1;
        }
        else if (delay == 0 && res < 7) {
            tn += 1;
        }
        else if (delay == 1 && res < 7) {
            fn += 1;
        }
        else {
            fp += 1;
        }
    }
    printf("Accuracy: %.4f\n", (double)(tp + tn) / (tp + tn + fp + fn));
    printf("tp = %d, tn = %d, fp = %d, fn = %d\n", tp, tn, fp, fn);
    
    return 0;
}