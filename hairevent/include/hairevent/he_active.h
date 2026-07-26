#ifndef HE_ACTIVE_H
#define HE_ACTIVE_H

#include <stdbool.h>
#include <stddef.h>

#include "hairtos/hr_status.h"
#include "hairtos/hr_task.h"
#include "he_event.h"
#include "he_state_machine.h"
#include "he_types.h"

hr_status_t he_active_create_static(he_active_t *active,
                                    const char *name,
                                    he_state_handler_t initial_state,
                                    void *context,
                                    he_event_t **queue_storage,
                                    size_t queue_capacity,
                                    hr_stack_t *stack,
                                    size_t stack_words,
                                    hr_priority_t priority);

hr_status_t he_active_post(he_active_t *active,
                           he_event_t *event,
                           hr_tick_t timeout);
hr_status_t he_active_post_from_isr(he_active_t *active,
                                    he_event_t *event,
                                    bool *higher_priority_task_woken);

bool he_active_is_valid(const he_active_t *active);
const char *he_active_get_name(const he_active_t *active);
size_t he_active_get_pending_count(const he_active_t *active);
hr_task_t *he_active_get_task(he_active_t *active);
he_state_machine_t *he_active_get_state_machine(he_active_t *active);

#endif /* HE_ACTIVE_H */
