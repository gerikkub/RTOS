#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <stdint.h>
#include <stdbool.h>

#include "task.h"

typedef struct {
    uint8_t task_list[MAX_TASKS];
    uint8_t idx;
} task_set;

void init_schedule();

void enter_schedule(uint32_t sp);
void schedule();

void insert_task_default(int task_num);
void insert_task_at_priority(int task_num, uint8_t priority);
void delist_task(int task_num);

bool create_task(task_f func, uint8_t priority, uint32_t stack_size, void* parameters);

task_s* current_task(void);

uint8_t current_task_num(void);

void panic(void);

void systick_handler(void);

#endif