#ifndef HR_TASK_H
#define HR_TASK_H

#include "hr_status.h"
#include "hr_types.h"

typedef enum
{
    HR_TASK_STATE_INVALID = 0,
    HR_TASK_STATE_CREATED,
    HR_TASK_STATE_READY,
    HR_TASK_STATE_RUNNING,
    HR_TASK_STATE_BLOCKED,
    HR_TASK_STATE_SUSPENDED
} hr_task_state_t;

hr_status_t hr_task_create_static(hr_task_t *task,
                                  const char *name,
                                  hr_task_entry_t entry,
                                  void *argument,
                                  hr_stack_t *stack,
                                  size_t stack_words,
                                  hr_priority_t priority);

bool hr_task_is_valid(const hr_task_t *task);
const char *hr_task_get_name(const hr_task_t *task);
hr_task_state_t hr_task_get_state(const hr_task_t *task);
hr_priority_t hr_task_get_base_priority(const hr_task_t *task);
hr_priority_t hr_task_get_effective_priority(const hr_task_t *task);
size_t hr_task_get_stack_words(const hr_task_t *task);
size_t hr_task_get_stack_high_watermark(const hr_task_t *task);
bool hr_task_stack_guard_is_valid(const hr_task_t *task);

/* Implemented in later phases. */
void hr_task_yield(void);
void hr_task_delay(hr_tick_t ticks);
void hr_task_delay_until(hr_tick_t *last_wake_tick, hr_tick_t period);
hr_task_t *hr_task_current(void);

#endif /* HR_TASK_H */
