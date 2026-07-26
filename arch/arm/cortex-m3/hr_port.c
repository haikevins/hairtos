#include <stdint.h>

#include "hr_port.h"
#include "hr_portmacro.h"

hr_stack_t *hr_port_initialize_stack(hr_stack_t *stack_low,
                                     size_t stack_words,
                                     hr_task_entry_t entry,
                                     void *argument,
                                     void (*exit_handler)(void))
{
    const uintptr_t entry_address = (uintptr_t)entry;
    const uintptr_t argument_address = (uintptr_t)argument;
    const uintptr_t exit_address = (uintptr_t)exit_handler;

    return hr_port_initialize_stack_raw(stack_low,
                                        stack_words,
                                        (uint32_t)entry_address,
                                        (uint32_t)argument_address,
                                        (uint32_t)exit_address);
}
