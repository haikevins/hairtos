#include <stdbool.h>
#include <stdint.h>

#include "hr_port.h"
#include "hr_portmacro.h"

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

void hr_port_wait_for_interrupt(void)
{
}

bool hr_port_thread_uses_psp(void)
{
    return false;
}
