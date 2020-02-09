#ifndef __TASK_H__
#define __TASK_H__

#include "config.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t calleeRegs[8];
    uint32_t lr_exception;
    uint32_t callerRegs[6];
    uint32_t return_address;
    uint32_t xPSR;
} TaskRegisterState;

typedef enum {
    TS_RUNNABLE,
    TS_WAIT_TIMER,
    TS_WAIT_ISR
} TaskState;

typedef struct {
    uintptr_t stack;
    uintptr_t sp;
    TaskState state;
    TaskRegisterState registers;
    uint8_t priority;
    uint8_t current_priority;
    uint8_t task_num;
    bool valid;
// Only valid in TS_WAIT_TIME state
    uint32_t timer_val;
} task_s;

typedef void (*task_f)(void*);

void setup_task_stack(task_s* task, task_f func, intptr_t stack_base, intptr_t parameters);

void run_task(task_s* task);

void run_task_asm(intptr_t sp, TaskRegisterState* registers);

void yield();

void yield_for(uint32_t ms);

void make_syscall(uint32_t syscall_number, uintptr_t args);

#endif