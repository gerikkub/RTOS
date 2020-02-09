
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


#include "syscall.h"
#include "schedule.h"
#include "isr_notify.h"

void syscall_wait_for_helper(void* void_args);
void syscall_isr_wait_helper(void* void_args);

void yield() {
    make_syscall(SYSCALL_YIELD, (uintptr_t)NULL);
}

void wait_for(uint32_t time_ms) {
    SyscallArgsWaitFor args = {
        .time_ms = time_ms
    };

    make_syscall(SYSCALL_WAIT, (uintptr_t)&args);
}

void isr_wait(uint32_t isr) {
    SyscallArgsIsrWait args = {
        .isr = isr
    };

    make_syscall(SYSCALL_ISR_WAIT, (uintptr_t)&args);
}

void syscall_handler(uint32_t type, void* args) {

    switch (type) {
        case SYSCALL_YIELD:
            break;
        case SYSCALL_WAIT:
            syscall_wait_for_helper(args);
            break;
        case SYSCALL_ISR_WAIT:
            syscall_isr_wait_helper(args);
            break;
        default:
            panic();
            break;
    }

    schedule();
}

void syscall_wait_for_helper(void* void_args) {

    SyscallArgsWaitFor* args = (SyscallArgsWaitFor*)void_args;

    task_s* this_task = current_task();

    this_task->timer_val = args->time_ms;
    this_task->state = TS_WAIT_TIMER;

    uint8_t this_task_num = current_task_num();
    delist_task(this_task_num);

    SEGGER_RTT_WriteString(0, "Wait syscall: ");
    char num_memory[16];
    snprintf(num_memory, sizeof(num_memory), "%d", args->time_ms);

    SEGGER_RTT_WriteString(0, num_memory);
    SEGGER_RTT_WriteString(0, "\r\n");
}

void syscall_isr_wait_helper(void* void_args) {

    SyscallArgsIsrWait* args = (SyscallArgsIsrWait*)void_args;

    task_s* this_task = current_task();

    // Need to disable interrupts for the check
    asm("cpsid i");

    if (isr_notify_get_val(args->isr) == 0) {

        this_task->state = TS_WAIT_ISR;
        isr_notify_wait_task(args->isr, this_task);

        uint8_t this_task_num = current_task_num();
        delist_task(this_task_num);

        SEGGER_RTT_WriteString(0, "IRQ Wait on: ");
        char num_memory[16];
        snprintf(num_memory, sizeof(num_memory), "%d", args->isr);

        SEGGER_RTT_WriteString(0, num_memory);
        SEGGER_RTT_WriteString(0, "\r\n");
    }

    asm("cpsie i");
}