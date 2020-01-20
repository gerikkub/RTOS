
#include <stdint.h>

#include "lock.h"

uint32_t compare_and_swap(volatile uint32_t* mem, uint32_t exp_val, uint32_t new_val) {
    asm("cpsid i");
    asm("dmb");

    uint32_t val = *mem;
    if (val == exp_val) {
        *mem = new_val;
    }

    asm("dmb");
    asm("cpsie i");
    return val;
}

void mutex_init(OsMutex* mutex) {
    mutex->val = 0;
}

void mutex_spinlock(OsMutex* mutex) {
    while (compare_and_swap(&mutex->val, 0, 1) ==  0)
    {}
}

void mutex_spinunlock(OsMutex* mutex) {
    mutex->val = 0;
    asm("dmb");
}
