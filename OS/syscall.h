#ifndef __SYSCALL_H__
#define __SYSCALL_H__

void yield();

void wait_for(uint32_t time_ms);

void syscall_handler(uint32_t type, void* args);

typedef struct {
    uint32_t time_ms;
} SyscallArgsWaitFor;

typedef struct {
    uint32_t isr;
} SyscallArgsIsrWait;

typedef enum {
    SYSCALL_YIELD = 0,
    SYSCALL_WAIT,
    SYSCALL_ISR_WAIT
} SyscallType;


#endif