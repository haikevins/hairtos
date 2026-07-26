#include <stdbool.h>
#include <stdint.h>

#include "hr_port.h"
#include "hr_portmacro.h"

#define HR_PORT_SCB_ICSR_ADDRESS         0xE000ED04UL
#define HR_PORT_SCB_CCR_ADDRESS          0xE000ED14UL
#define HR_PORT_SCB_SHP_ADDRESS          0xE000ED18UL
#define HR_PORT_SCB_SHCSR_ADDRESS        0xE000ED24UL
#define HR_PORT_SYSTICK_VAL_ADDRESS       0xE000E018UL
#define HR_PORT_SCB_ICSR_PENDSVSET       (UINT32_C(1) << 28U)
#define HR_PORT_SCB_CCR_STKALIGN         (UINT32_C(1) << 9U)
#define HR_PORT_SCB_CCR_UNALIGN_TRP      (UINT32_C(1) << 3U)
#define HR_PORT_SCB_CCR_DIV_0_TRP        (UINT32_C(1) << 4U)
#define HR_PORT_SCB_SHCSR_MEMFAULTENA    (UINT32_C(1) << 16U)
#define HR_PORT_SCB_SHCSR_BUSFAULTENA    (UINT32_C(1) << 17U)
#define HR_PORT_SCB_SHCSR_USGFAULTENA    (UINT32_C(1) << 18U)
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
#if (HR_CFG_ENABLE_DIAGNOSTICS == 1)
    HR_PORT_REG32(HR_PORT_SCB_CCR_ADDRESS) |=
        HR_PORT_SCB_CCR_UNALIGN_TRP | HR_PORT_SCB_CCR_DIV_0_TRP;
    HR_PORT_REG32(HR_PORT_SCB_SHCSR_ADDRESS) |=
        HR_PORT_SCB_SHCSR_MEMFAULTENA | HR_PORT_SCB_SHCSR_BUSFAULTENA |
        HR_PORT_SCB_SHCSR_USGFAULTENA;
#endif
    priorities[HR_PORT_SHP_SVC_INDEX] = HR_PORT_PRIORITY_HIGHEST;
    priorities[HR_PORT_SHP_PENDSV_INDEX] = HR_PORT_PRIORITY_LOWEST;
    priorities[HR_PORT_SHP_SYSTICK_INDEX] = HR_PORT_PRIORITY_MEDIUM;
    HR_PORT_REG32(HR_PORT_SYSTICK_VAL_ADDRESS) = 0U;

    __asm volatile ("dsb" ::: "memory");
    __asm volatile ("isb" ::: "memory");
}


void hr_port_request_context_switch(void)
{
    HR_PORT_REG32(HR_PORT_SCB_ICSR_ADDRESS) = HR_PORT_SCB_ICSR_PENDSVSET;
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

hr_irq_state_t hr_port_enter_critical(void)
{
    hr_irq_state_t state;

    __asm volatile ("mrs %0, primask" : "=r"(state));
    __asm volatile ("cpsid i" ::: "memory");
    __asm volatile ("dsb" ::: "memory");
    __asm volatile ("isb" ::: "memory");
    return state;
}

void hr_port_exit_critical(hr_irq_state_t state)
{
    __asm volatile ("msr primask, %0" :: "r"(state) : "memory");
    __asm volatile ("dsb" ::: "memory");
    __asm volatile ("isb" ::: "memory");
}

bool hr_port_is_inside_isr(void)
{
    uint32_t ipsr;

    __asm volatile ("mrs %0, ipsr" : "=r"(ipsr));
    return ipsr != 0U;
}

void hr_port_yield_from_isr(bool required)
{
    if (required)
    {
        hr_port_request_context_switch();
    }
}
