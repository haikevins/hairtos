#ifndef HR_PORT_H
#define HR_PORT_H

#include <stddef.h>

#include "hairtos/hr_types.h"

hr_stack_t *hr_port_initialize_stack(hr_stack_t *stack_low,
                                     size_t stack_words,
                                     hr_task_entry_t entry,
                                     void *argument,
                                     void (*exit_handler)(void));

/* Implemented in later phases. */
void hr_port_start_first_task(void);
void hr_port_request_context_switch(void);
hr_irq_state_t hr_port_enter_critical(void);
void hr_port_exit_critical(hr_irq_state_t state);
bool hr_port_is_inside_isr(void);
void hr_port_yield_from_isr(bool required);

#endif /* HR_PORT_H */
