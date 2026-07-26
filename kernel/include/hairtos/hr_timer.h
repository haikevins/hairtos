#ifndef HR_TIMER_H
#define HR_TIMER_H

#include <stdbool.h>
#include <stdint.h>

#include "hr_status.h"
#include "hr_types.h"

hr_status_t hr_timer_create_static(hr_timer_t *timer,
                                   const char *name,
                                   hr_tick_t period_ticks,
                                   bool auto_reload,
                                   hr_timer_callback_t callback,
                                   void *argument);

bool hr_timer_is_valid(const hr_timer_t *timer);
bool hr_timer_is_active(const hr_timer_t *timer);
const char *hr_timer_get_name(const hr_timer_t *timer);
hr_tick_t hr_timer_get_period(const hr_timer_t *timer);
uint32_t hr_timer_get_pending_count(const hr_timer_t *timer);

hr_status_t hr_timer_start(hr_timer_t *timer);
hr_status_t hr_timer_stop(hr_timer_t *timer);
hr_status_t hr_timer_reset(hr_timer_t *timer);
hr_status_t hr_timer_change_period(hr_timer_t *timer,
                                   hr_tick_t new_period_ticks);

#endif /* HR_TIMER_H */
