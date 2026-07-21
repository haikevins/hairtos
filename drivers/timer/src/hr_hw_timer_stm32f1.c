#include "hr_hw_timer.h"
#include "stm32f1.h"

static volatile uint32_t g_millisecond_tick;

void hr_hw_timer_init_1khz(uint32_t core_clock_hz)
{
    uint32_t reload;

    if (core_clock_hz < 1000UL)
    {
        return;
    }

    reload = (core_clock_hz / 1000UL) - 1UL;
    if (reload > SYSTICK_LOAD_RELOAD_MASK)
    {
        return;
    }

    g_millisecond_tick = 0UL;
    STM32F1_SYSTICK->CTRL = 0UL;
    STM32F1_SYSTICK->LOAD = reload;
    STM32F1_SYSTICK->VAL = 0UL;
    STM32F1_SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE |
                            SYSTICK_CTRL_TICKINT |
                            SYSTICK_CTRL_ENABLE;
}

uint32_t hr_hw_timer_millis(void)
{
    return g_millisecond_tick;
}

void hr_hw_timer_delay_ms(uint32_t milliseconds)
{
    const uint32_t start = g_millisecond_tick;

    while ((uint32_t)(g_millisecond_tick - start) < milliseconds)
    {
        stm32f1_cpu_wait_for_interrupt();
    }
}

void SysTick_Handler(void)
{
    g_millisecond_tick++;
}
