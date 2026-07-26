#ifndef HE_TIME_EVENT_H
#define HE_TIME_EVENT_H

#include <stdbool.h>
#include <stdint.h>

#include "hairtos/hr_status.h"
#include "he_active.h"
#include "he_types.h"

hr_status_t he_time_event_create_static(he_time_event_t *time_event,
                                        const char *name,
                                        he_active_t *target,
                                        he_signal_t signal,
                                        hr_tick_t period_ticks,
                                        bool periodic);
hr_status_t he_time_event_arm(he_time_event_t *time_event);
hr_status_t he_time_event_disarm(he_time_event_t *time_event);
hr_status_t he_time_event_rearm(he_time_event_t *time_event);
hr_status_t he_time_event_change_period(he_time_event_t *time_event,
                                        hr_tick_t period_ticks);

bool he_time_event_is_valid(const he_time_event_t *time_event);
bool he_time_event_is_armed(const he_time_event_t *time_event);
uint32_t he_time_event_get_dropped_count(const he_time_event_t *time_event);

#endif /* HE_TIME_EVENT_H */
