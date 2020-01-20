

#include "schedule.h"
#include "task.h"
#include "syscall.h"
#include "rtt_assert.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

extern intptr_t _ebss;
extern intptr_t _sidata;
extern intptr_t _etext;

void setup_task_stack(task_s* task, task_f func, intptr_t stack_base, intptr_t parameters) {
    task->registers.return_address = (uintptr_t)func;
    task->registers.callerRegs[0] = parameters; // R0
    task->registers.xPSR = (1 << 24); // Set the T bit only (thumb mode)
    task->registers.lr_exception = 0xFFFFFFF9; // Return to thread mode with Main stack

    task->sp = stack_base - sizeof(TaskRegisterState);

    memcpy((void*)task->sp, &task->registers, sizeof(TaskRegisterState));
}

void run_task(task_s* task) {
    ASSERT2(task->sp <= task->stack, task->sp, task->stack);
    ASSERT2(task->sp > _sidata, task->sp, _sidata);
    ASSERT1(task->registers.lr_exception & 0xFFFFFFF0 == 0xFFFFFFF0, task->registers.lr_exception);
    ASSERT2(task->registers.return_address < (0x08000000 + 256*1024), task->registers.return_address, _etext);
    ASSERT1(task->registers.return_address > 0x08000000, task->registers.return_address);

    run_task_asm(task->sp, &task->registers);
}

void systick_save_task(TaskRegisterState* register_state) {
    task_s* this_task = current_task();

    if (this_task != NULL) {
        this_task->registers = *register_state;
        this_task->sp = ((uintptr_t)register_state);
        //ASSERT2(this_task->sp <= this_task->stack, this_task->sp, this_task->stack);
    }

    systick_handler();
}

void syscall_save_task(TaskRegisterState* register_state) {
    task_s* this_task = current_task();

    if (this_task != NULL) {
        this_task->registers = *register_state;
        this_task->sp = ((uintptr_t)register_state);
        ASSERT2(this_task->sp <= this_task->stack, this_task->sp, this_task->stack);
    }

    syscall_handler(register_state->callerRegs[0],
                    (void*)register_state->callerRegs[1]);

}