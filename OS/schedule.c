
#include "config.h"
#include "schedule.h"
#include "task.h"
#include "RTT/SEGGER_RTT.h"
#include "rtt_assert.h"

#include "stm32f30x.h"
#include "core_cm4.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#define ELEMENTS(X) \
(sizeof(X) / sizeof(X[0]))

static volatile task_s g_task_list[MAX_TASKS];
static volatile uint32_t g_last_task_idx = 0;

static volatile task_set g_priority_table[MAX_PRIORITIES];

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

    for (i = 0; i < MAX_PRIORITIES; i++) {
        int j;
        for (j = 0; j < MAX_TASKS; j++) {
            g_priority_table[i].task_list[j] = 0xFF;
        }
        g_priority_table[i].idx = 0;
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

    bool found_task = false;
    for (i = 0; i < MAX_PRIORITIES && !found_task; i++) {
        uint8_t* tasks = g_priority_table[i].task_list;

        int task_idx = g_priority_table[i].idx;

        if (tasks[task_idx] != 0xFF) {
            found_task = true;
            next_candidate = tasks[task_idx];
            g_priority_table[i].idx = (task_idx + 1) % MAX_TASKS;
        } else {
            task_idx = 0;
            if (tasks[task_idx] != 0xFF) {
                found_task = true;
                next_candidate = tasks[task_idx];
                g_priority_table[i].idx = 1;
            } else {
                int j;
                for (j = 0; j < MAX_TASKS; j++) {
                    ASSERT2(tasks[j] == 0xFF, j, tasks[j]);
                }

                g_priority_table[i].idx = 0;
            }
        }
    }

    if (found_task) {
        ASSERT1(g_task_list[next_candidate].valid, next_candidate);
        ASSERT1(g_task_list[next_candidate].state == TS_RUNNABLE, next_candidate);

        g_last_task_idx = next_candidate;

        SEGGER_RTT_WriteNoLock(0, "\r\nSchedule: ", 12);
        SEGGER_RTT_PutCharSkipNoLock(0, hex_table[next_candidate]);

        SysTick->VAL = 0;

        run_task(&g_task_list[next_candidate]);
    } else {
        SEGGER_RTT_WriteNoLock(0, "\r\nSchedule: IDLE", 16);
        run_idle_task();
    }

}

void insert_task_default(int task_num) {
    ASSERT1(task_num < MAX_TASKS, task_num);

    uint8_t priority = g_task_list[task_num].priority;

    ASSERT1(priority < MAX_PRIORITIES, priority);

    insert_task_at_priority(task_num, priority);
}

void insert_task_at_priority(int task_num, uint8_t priority) {

    uint8_t* tasks = g_priority_table[priority].task_list;

    int i;
    for (i = 0; i < MAX_TASKS; i++) {
        if (tasks[i] == 0xFF) {
            tasks[i] = task_num;
            break;
        }
    }

    ASSERT2(i < MAX_TASKS, task_num, priority);
}

void delist_task(int task_num) {
    uint8_t current_priority = g_task_list[task_num].current_priority;

    uint8_t* tasks = g_priority_table[current_priority].task_list;

    int i;
    bool found_task = false;
    for (i = 0; i < MAX_TASKS; i++) {
        if (!found_task) {
            if (tasks[i] == task_num) {
                found_task = true;
                tasks[i] = 0xFF;
            }
        } else {
            tasks[i-1] = tasks[i];

            if (tasks[i] == 0xFF) {
                break;
            }
        }
    }

    ASSERT(found_task);
}

bool create_task(task_f func, uint8_t priority, uint32_t task_stack_size, void* parameters) {

    int task_num;
    for (task_num = 0; task_num < ELEMENTS(g_task_list); task_num++) {
        if (!g_task_list[task_num].valid) {
            break;
        }
    }

    if (task_num == ELEMENTS(g_task_list)) {
        return false;
    }

    if (g_stack_mem_idx < task_stack_size) {
        return false;
    }

    task_s* this_task = &g_task_list[task_num];

    uint8_t* stack_base = &g_stack_mem[g_stack_mem_idx];

    setup_task_stack(this_task,
                     func,
                     (intptr_t)stack_base,
                     (intptr_t)parameters);

    this_task->stack = (intptr_t)stack_base;
    this_task->valid = true;
    this_task->state = TS_RUNNABLE;
    this_task->priority = priority;
    this_task->current_priority = priority;
    this_task->task_num = task_num;

    g_stack_mem_idx -= task_stack_size;

    insert_task_default(task_num);

    return true;
}

task_s* current_task(void) {
    if (g_last_task_idx == 0xFFFFFFFF) {
        return NULL;
    } else {
        return &g_task_list[g_last_task_idx];
    }
}

uint8_t current_task_num(void) {
    if (g_last_task_idx == 0xFFFFFFFF) {
        return 0xFF;
    } else {
        return g_last_task_idx;
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
                insert_task_default(g_task_list[i].task_num);
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