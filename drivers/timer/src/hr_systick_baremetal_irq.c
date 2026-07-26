#include "hr_hw_timer.h"

void SysTick_Handler(void)
{
    hr_hw_timer_tick_isr();
}
