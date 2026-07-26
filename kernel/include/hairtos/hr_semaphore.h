#ifndef HR_SEMAPHORE_H
#define HR_SEMAPHORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hr_status.h"
#include "hr_types.h"

hr_status_t hr_semaphore_create_counting(hr_semaphore_t *semaphore,
                                         uint32_t initial_count,
                                         uint32_t max_count);
hr_status_t hr_semaphore_create_binary(hr_semaphore_t *semaphore,
                                       bool initially_available);

bool hr_semaphore_is_valid(const hr_semaphore_t *semaphore);
uint32_t hr_semaphore_get_count(const hr_semaphore_t *semaphore);
uint32_t hr_semaphore_get_max_count(const hr_semaphore_t *semaphore);
size_t hr_semaphore_get_waiting_tasks(const hr_semaphore_t *semaphore);

hr_status_t hr_semaphore_take(hr_semaphore_t *semaphore, hr_tick_t timeout);
hr_status_t hr_semaphore_give(hr_semaphore_t *semaphore);
hr_status_t hr_semaphore_give_from_isr(hr_semaphore_t *semaphore,
                                       bool *higher_priority_task_woken);

#endif /* HR_SEMAPHORE_H */
