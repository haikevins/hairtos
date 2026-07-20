#ifndef HR_MUTEX_H
#define HR_MUTEX_H
#include "hr_status.h"
#include "hr_types.h"
hr_status_t hr_mutex_create(hr_mutex_t *mutex);
hr_status_t hr_mutex_lock(hr_mutex_t *mutex,hr_tick_t timeout);
hr_status_t hr_mutex_unlock(hr_mutex_t *mutex);
#endif
