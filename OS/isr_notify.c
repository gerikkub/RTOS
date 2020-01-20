
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "isr_notify.h"
#include "task.h"
#include "rtt_assert.h"
#include "lock.h"

#define MAX_NUM_IRQ 256

static volatile uint32_t g_notify_counts[MAX_NUM_IRQ] = {0};

static volatile task_s* g_notify_tasks[MAX_NUM_IRQ] = {0};

void isr_notify_try_wait(uint32_t isr) {
    ASSERT1(isr < MAX_NUM_IRQ, isr);

    uint32_t count, new_count;
    while (true) {
       count = g_notify_counts[isr]; 

        if (count > 0) {
            new_count = count - 1;
            if (compare_and_swap(&g_notify_counts[isr], count, new_count) == count) {
                break;
            }
        } else {
            // No calls. Need to wait in the kernel
            isr_wait(isr);
            break;
        }
    }
}

void isr_notify_clear(uint32_t isr) {
    ASSERT1(isr < MAX_NUM_IRQ, isr);

    g_notify_counts[isr] = 0;
}

void isr_notify_notify_ISR(uint32_t isr) {
    ASSERT1(isr < MAX_NUM_IRQ, isr);

    // ASSERT in IRQ context

    if (g_notify_tasks[isr] != NULL) {
        g_notify_tasks[isr]->state = TS_RUNNABLE;
        g_notify_tasks[isr] = NULL;
    } else {
        g_notify_counts[isr]++;
    }
}

uint32_t isr_notify_get_val(uint32_t isr) {
    ASSERT1(isr < MAX_NUM_IRQ, isr);

    return g_notify_counts[isr];
}

void isr_notify_wait_task(uint32_t isr, task_s* task) {
    ASSERT1(isr < MAX_NUM_IRQ, isr);
    ASSERT(task != NULL);

    g_notify_tasks[isr] = task;
}