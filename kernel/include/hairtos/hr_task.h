#ifndef HR_TASK_H
#define HR_TASK_H
#include "hr_status.h"
#include "hr_types.h"
hr_status_t hr_task_create_static(hr_task_t *task,const char *name,hr_task_entry_t entry,
void *argument,hr_stack_t *stack,size_t stack_words,hr_priority_t priority);
void hr_task_yield(void);
void hr_task_delay(hr_tick_t ticks);
void hr_task_delay_until(hr_tick_t *last_wake_tick,hr_tick_t period);
hr_task_t *hr_task_current(void);
#endif
