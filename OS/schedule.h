#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <stdint.h>
#include <stdbool.h>

#include "task.h"

void init_schedule();

void enter_schedule(uint32_t sp);
void schedule();

bool create_task(task_f func, uint32_t stack_size, void* parameters);

task_s* current_task(void);

void panic(void);

void systick_handler(void);

#endif