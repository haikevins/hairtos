#include "hairtos/hr_context.h"
#include "hr_port.h"

hr_irq_state_t hr_critical_enter(void)
{
    return hr_port_enter_critical();
}

void hr_critical_exit(hr_irq_state_t state)
{
    hr_port_exit_critical(state);
}

bool hr_is_inside_isr(void)
{
    return hr_port_is_inside_isr();
}

void hr_yield_from_isr(bool switch_required)
{
    hr_port_yield_from_isr(switch_required);
}
