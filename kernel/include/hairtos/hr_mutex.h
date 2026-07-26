#ifndef HR_MUTEX_H
#define HR_MUTEX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hr_status.h"
#include "hr_types.h"

hr_status_t hr_mutex_create(hr_mutex_t *mutex);
hr_status_t hr_mutex_create_recursive(hr_mutex_t *mutex);

bool hr_mutex_is_valid(const hr_mutex_t *mutex);
bool hr_mutex_is_recursive(const hr_mutex_t *mutex);
hr_task_t *hr_mutex_get_owner(const hr_mutex_t *mutex);
uint32_t hr_mutex_get_recursion_count(const hr_mutex_t *mutex);
size_t hr_mutex_get_waiting_tasks(const hr_mutex_t *mutex);

hr_status_t hr_mutex_lock(hr_mutex_t *mutex, hr_tick_t timeout);
hr_status_t hr_mutex_unlock(hr_mutex_t *mutex);

#endif /* HR_MUTEX_H */
