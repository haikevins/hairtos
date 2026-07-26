#ifndef HE_STATE_MACHINE_H
#define HE_STATE_MACHINE_H

#include <stdbool.h>

#include "hairtos/hr_status.h"
#include "he_event.h"
#include "he_types.h"

typedef enum
{
    HE_STATE_HANDLED = 0,
    HE_STATE_IGNORED,
    HE_STATE_TRANSITION
} he_state_result_t;

typedef he_state_result_t (*he_state_handler_t)(he_state_machine_t *machine,
                                                const he_event_t *event);

hr_status_t he_state_machine_init(he_state_machine_t *machine,
                                  he_state_handler_t initial_state,
                                  void *context);
hr_status_t he_state_machine_start(he_state_machine_t *machine);
hr_status_t he_state_machine_dispatch(he_state_machine_t *machine,
                                      const he_event_t *event);
he_state_result_t he_state_transition(he_state_machine_t *machine,
                                      he_state_handler_t target_state);

bool he_state_machine_is_valid(const he_state_machine_t *machine);
bool he_state_machine_is_started(const he_state_machine_t *machine);
he_state_handler_t he_state_machine_current(const he_state_machine_t *machine);
void *he_state_machine_context(he_state_machine_t *machine);
const void *he_state_machine_context_const(const he_state_machine_t *machine);

#endif /* HE_STATE_MACHINE_H */
