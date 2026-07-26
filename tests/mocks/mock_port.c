#include <stdbool.h>
#include <stdint.h>

#include "hr_port.h"
#include "hr_portmacro.h"

unsigned int g_mock_context_switch_requests = 0U;
bool g_mock_inside_isr = false;
static hr_irq_state_t g_mock_irq_state = 0U;

hr_stack_t *hr_port_initialize_stack(hr_stack_t *stack_low,
                                     size_t stack_words,
                                     hr_task_entry_t entry,
                                     void *argument,
                                     void (*exit_handler)(void))
{
    (void)entry;
    (void)argument;
    (void)exit_handler;

    return hr_port_initialize_stack_raw(stack_low,
                                        stack_words,
                                        0x08001235UL,
                                        0x20000100UL,
                                        0x08000457UL);
}

void hr_port_configure_kernel_exceptions(void)
{
}

void hr_port_start_first_task(void)
{
}

void hr_port_request_context_switch(void)
{
    g_mock_context_switch_requests++;
}

void hr_port_wait_for_interrupt(void)
{
}

bool hr_port_thread_uses_psp(void)
{
    return false;
}

hr_irq_state_t hr_port_enter_critical(void)
{
    const hr_irq_state_t previous = g_mock_irq_state;
    g_mock_irq_state = 1U;
    return previous;
}

void hr_port_exit_critical(hr_irq_state_t state)
{
    g_mock_irq_state = state;
}

bool hr_port_is_inside_isr(void)
{
    return g_mock_inside_isr;
}

void hr_port_yield_from_isr(bool required)
{
    if (required)
    {
        hr_port_request_context_switch();
    }
}
