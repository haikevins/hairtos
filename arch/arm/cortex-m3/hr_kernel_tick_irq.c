
#include "hr_kernel_internal.h"

void SysTick_Handler(void)
{
    hr_kernel_tick_from_isr();
}
