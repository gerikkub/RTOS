#ifndef __ISR_NOTIFY_H__
#define __ISR_NOTIFY_H__

#include <stdint.h>

#include "task.h"

void isr_notify_try_wait(uint32_t isr);

void isr_notify_clear(uint32_t isr);

void isr_notify_notify_ISR(uint32_t isr);

uint32_t isr_notify_get_val(uint32_t isr);

void isr_notify_wait_task(uint32_t isr, task_s* task);


#endif