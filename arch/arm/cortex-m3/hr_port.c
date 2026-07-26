#include <stdbool.h>
#include <stdint.h>

#include "hr_port.h"
#include "hr_portmacro.h"

#define HR_PORT_SCB_CCR_ADDRESS          0xE000ED14UL
#define HR_PORT_SCB_SHP_ADDRESS          0xE000ED18UL
#define HR_PORT_SCB_CCR_STKALIGN         (UINT32_C(1) << 9U)
#define HR_PORT_SHP_SVC_INDEX            7U
#define HR_PORT_SHP_PENDSV_INDEX         10U
#define HR_PORT_SHP_SYSTICK_INDEX        11U
#define HR_PORT_PRIORITY_HIGHEST         0x00U
#define HR_PORT_PRIORITY_MEDIUM          0x80U
#define HR_PORT_PRIORITY_LOWEST          0xFFU

#define HR_PORT_REG32(address) (*(volatile uint32_t *)(uintptr_t)(address))
#define HR_PORT_REG8_PTR(address) ((volatile uint8_t *)(uintptr_t)(address))

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

void hr_port_configure_kernel_exceptions(void)
{
    volatile uint8_t *const priorities = HR_PORT_REG8_PTR(HR_PORT_SCB_SHP_ADDRESS);

    HR_PORT_REG32(HR_PORT_SCB_CCR_ADDRESS) |= HR_PORT_SCB_CCR_STKALIGN;
    priorities[HR_PORT_SHP_SVC_INDEX] = HR_PORT_PRIORITY_HIGHEST;
    priorities[HR_PORT_SHP_PENDSV_INDEX] = HR_PORT_PRIORITY_LOWEST;
    priorities[HR_PORT_SHP_SYSTICK_INDEX] = HR_PORT_PRIORITY_MEDIUM;

    __asm volatile ("dsb" ::: "memory");
    __asm volatile ("isb" ::: "memory");
}

void hr_port_wait_for_interrupt(void)
{
    __asm volatile ("wfi" ::: "memory");
}

bool hr_port_thread_uses_psp(void)
{
    uint32_t control;

    __asm volatile ("mrs %0, control" : "=r"(control));
    return (control & UINT32_C(2)) != 0U;
}
