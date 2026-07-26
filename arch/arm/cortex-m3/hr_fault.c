#include <stddef.h>
#include <stdint.h>

#include "hairtos/hr_diagnostics.h"
#include "hr_diagnostics_internal.h"

#define HR_FAULT_REG32(address) (*(volatile uint32_t *)(uintptr_t)(address))
#define HR_FAULT_SCB_CFSR       0xE000ED28UL
#define HR_FAULT_SCB_HFSR       0xE000ED2CUL
#define HR_FAULT_SCB_DFSR       0xE000ED30UL
#define HR_FAULT_SCB_MMFAR      0xE000ED34UL
#define HR_FAULT_SCB_BFAR       0xE000ED38UL
#define HR_FAULT_SCB_AFSR       0xE000ED3CUL
#define HR_FAULT_SCB_SHCSR      0xE000ED24UL

_Noreturn void hr_port_fault_capture(const uint32_t *stack_frame,
                                     uint32_t exception_return,
                                     uint32_t reason_value)
{
    hr_fault_context_t fault;
    uint32_t exception_number;
    unsigned char *fault_bytes = (unsigned char *)&fault;
    size_t index;

    for (index = 0U; index < sizeof(fault); index++)
    {
        fault_bytes[index] = 0U;
    }

    __asm volatile ("cpsid i" ::: "memory");
    __asm volatile ("mrs %0, ipsr" : "=r"(exception_number));

    if (stack_frame != NULL)
    {
        fault.r0 = stack_frame[0];
        fault.r1 = stack_frame[1];
        fault.r2 = stack_frame[2];
        fault.r3 = stack_frame[3];
        fault.r12 = stack_frame[4];
        fault.lr = stack_frame[5];
        fault.pc = stack_frame[6];
        fault.xpsr = stack_frame[7];
    }

    fault.exception_return = exception_return;
    fault.exception_number = exception_number;
    fault.cfsr = HR_FAULT_REG32(HR_FAULT_SCB_CFSR);
    fault.hfsr = HR_FAULT_REG32(HR_FAULT_SCB_HFSR);
    fault.dfsr = HR_FAULT_REG32(HR_FAULT_SCB_DFSR);
    fault.afsr = HR_FAULT_REG32(HR_FAULT_SCB_AFSR);
    fault.mmfar = HR_FAULT_REG32(HR_FAULT_SCB_MMFAR);
    fault.bfar = HR_FAULT_REG32(HR_FAULT_SCB_BFAR);
    fault.shcsr = HR_FAULT_REG32(HR_FAULT_SCB_SHCSR);

    hr_diagnostics_record_fault((hr_panic_reason_t)reason_value, &fault);
    hr_diagnostics_platform_halt();
}

_Noreturn void hr_diagnostics_platform_halt(void)
{
    __asm volatile ("cpsid i" ::: "memory");
    for (;;)
    {
        __asm volatile ("bkpt #0");
    }
}
