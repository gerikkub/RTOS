


.syntax unified
.thumb


.global run_task_asm
.global syscall_asm
.global make_syscall
.global systick_asm

.extern enter_schedule
.word _estack
.word g_sched_stack_base

@ setup_task_stack:
@ # r0: function pointer
@ # r1: task stack base
@ # r2: tack parameter pointer

@ # Setup function address for entry
@     stmdb r1!, {r0}

@ # Add dummy register room
@     sub r1, r1, #32

@ # Store xPSR register
@     ...

@ # Store Return address
@     stmdb r1!, {r0}

@ # Skip for r12,r3,r2,r1
@     sub r1, r1, #16

@ # Store parameters
@     stmdb r1!, {r2}

@ # Return sp start
@     mov r0, r1

@     bx lr

run_task_asm:
# r0: Stack pointer
# r1: Pointer to stack registers
# Restore stack pointer
    mov sp, r0

    ldmia sp!,{r4-r11, lr}

    bx lr

@     add r2, r1, #36

@ # Grab the register that should reside on the stack for exception return
@ # And place them on the stack
@     ldmia r2,{r5-r12}
@     stmdb sp!,{r5-r12}

@ # Grab the callee saved registers
@     ldmia r1,{r4-r11,lr}

@ # Return from the exception
@     bx lr

systick_asm:
# Save r4-r11, lr on the stack
    stmdb sp!, {r4-r11, lr}

# Copy current stack pointer
    mov r0, sp

# Setup kernel stack pointer
    ldr r1, =g_sched_stack_base
    mov sp, r1

# Call the generic syscall handler
    blx systick_save_task


syscall_asm:
# Save r4-r11, lr on the stack
    stmdb sp!, {r4-r11, lr}

# Copy current stack pointer
    mov r0, sp

# Setup kernel stack pointer
    ldr r1, =g_sched_stack_base
    mov sp, r1

# Call the generic syscall handler
    blx syscall_save_task

make_syscall:
# r0: syscall number
# r1: argument pointer

    svc #0

    bx lr


