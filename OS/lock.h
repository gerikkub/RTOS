#ifndef __LOCK_H__
#define __LOCK_H__

typedef struct {
    volatile uint32_t val;
} OsMutex;

uint32_t compare_and_swap(volatile uint32_t* mem, uint32_t exp_val, uint32_t new_val);

void mutex_init(OsMutex* mutex);

void mutex_spinlock(OsMutex* mutex);

void mutex_spinunlock(OsMutex* mutex);

#endif