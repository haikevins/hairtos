#ifndef HR_SEMAPHORE_H
#define HR_SEMAPHORE_H
#include "hr_status.h"
#include "hr_types.h"
hr_status_t hr_semaphore_create_binary(hr_semaphore_t *semaphore,bool initially_available);
hr_status_t hr_semaphore_take(hr_semaphore_t *semaphore,hr_tick_t timeout);
hr_status_t hr_semaphore_give(hr_semaphore_t *semaphore);
hr_status_t hr_semaphore_give_from_isr(hr_semaphore_t *semaphore,bool *higher_priority_task_woken);
#endif
