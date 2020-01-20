
#include "schedule.h"
#include "task.h"
#include "RTT/SEGGER_RTT.h"

#include "stm32f30x.h"
#include "core_cm4.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#define MAX_TASKS 8
#define STACK_MEM_SIZE 8192

#define ELEMENTS(X) \
(sizeof(X) / sizeof(X[0]))

static volatile task_s g_task_list[MAX_TASKS];
static volatile uint32_t g_last_task_idx = 0;

static uint8_t g_stack_mem[STACK_MEM_SIZE];
static uint32_t g_stack_mem_idx = STACK_MEM_SIZE;

static uint8_t g_idle_task_stack[256];
static task_s g_idle_task;

uint8_t g_sched_stack[512];
intptr_t g_sched_stack_base;

void idle_task_func(void* params) {
    while(1)
    {
        asm("wfi");
        yield();
    }
}

void init_schedule() {

    int i;
    for (i = 0; i < ELEMENTS(g_task_list); i++) {
        g_task_list[i].valid = false;
    }

    g_last_task_idx = 0xFFFFFFFF;

    g_stack_mem_idx = ELEMENTS(g_stack_mem);
    for (i = 0; i < ELEMENTS(g_stack_mem); i++) {
        g_stack_mem[i] = 0xA5;
    }

    g_sched_stack_base = (intptr_t)&g_sched_stack[ELEMENTS(g_sched_stack)];

    SEGGER_RTT_Init();
}

void run_idle_task() {
    setup_task_stack(&g_idle_task,
                     idle_task_func,
                     (intptr_t)&g_idle_task_stack[ELEMENTS(g_idle_task_stack)], // Technically UB
                     (intptr_t)NULL);

    g_idle_task.stack = (intptr_t)&g_idle_task_stack[ELEMENTS(g_idle_task_stack)];
    g_idle_task.state = TS_RUNNABLE;
    g_idle_task.valid = true;

    g_last_task_idx = 0xFFFFFFFF;

    run_task(&g_idle_task);
}

void schedule() {

    const char hex_table[] = "0123456789ABCDEF";
    uint32_t next_candidate;
    int i;

    for (i = 0; i < ELEMENTS(g_task_list); i++) {
        next_candidate = (g_last_task_idx + 1 + i) % ELEMENTS(g_task_list);
        if (g_task_list[next_candidate].valid &&
            g_task_list[next_candidate].state == TS_RUNNABLE) {

            g_last_task_idx = next_candidate;
            //SEGGER_RTT_WriteNoLock(0, "\r\nSchedule: ", 12);
            //SEGGER_RTT_PutCharSkipNoLock(0, hex_table[next_candidate]);

            // Reset SysTick
            SysTick->VAL = 0;

            run_task(&g_task_list[next_candidate]);
        }
    }

    SEGGER_RTT_WriteNoLock(0, "\r\nSchedule: IDLE", 16);
    run_idle_task();
}

bool create_task(task_f func, uint32_t task_stack_size, void* parameters) {

    int i;
    for (i = 0; i < ELEMENTS(g_task_list); i++) {
        if (!g_task_list[i].valid) {
            break;
        }
    }

    if (i == ELEMENTS(g_task_list)) {
        return false;
    }

    if (g_stack_mem_idx < task_stack_size) {
        return false;
    }

    task_s* this_task = &g_task_list[i];

    uint8_t* stack_base = &g_stack_mem[g_stack_mem_idx];

    setup_task_stack(this_task,
                     func,
                     (intptr_t)stack_base,
                     (intptr_t)parameters);

    this_task->stack = (intptr_t)stack_base;
    this_task->valid = true;
    this_task->state = TS_RUNNABLE;

    g_stack_mem_idx -= task_stack_size;

    return true;
}

task_s* current_task(void) {
    if (g_last_task_idx == 0xFFFFFFFF) {
        return NULL;
    } else {
        return &g_task_list[g_last_task_idx];
    }
}

void systick_handler(void) {

    int i;
    for (i = 0; i < ELEMENTS(g_task_list); i++) {
        if (g_task_list[i].valid &&
            g_task_list[i].state == TS_WAIT_TIMER) {
                
            g_task_list[i].timer_val -= 1;

            if (g_task_list[i].timer_val == 0) {
                g_task_list[i].state = TS_RUNNABLE;
            }
        }
    }

    schedule();
}


void panic(void) {
    asm("cpsid i");
    asm("bkpt");
    while (1)
    {}
}