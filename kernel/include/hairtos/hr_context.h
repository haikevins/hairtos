#ifndef HR_CONTEXT_H
#define HR_CONTEXT_H

#include <stdbool.h>

#include "hr_types.h"

/* Architecture-neutral context helpers for framework and adapter layers. */
hr_irq_state_t hr_critical_enter(void);
void hr_critical_exit(hr_irq_state_t state);
bool hr_is_inside_isr(void);
void hr_yield_from_isr(bool switch_required);

#endif /* HR_CONTEXT_H */
