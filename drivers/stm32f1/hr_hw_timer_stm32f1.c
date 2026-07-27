
#include "hr_hw_timer.h"
#include "stm32f1.h"

static volatile uint32_t g_millisecond_tick;
static uint32_t g_tick_rate_hz;

bool hr_hw_timer_init(uint32_t tick_rate_hz)
{
    uint32_t reload;

    if ((tick_rate_hz == 0UL) || (SystemCoreClock < tick_rate_hz))
    {
        return false;
    }

    reload = (SystemCoreClock / tick_rate_hz) - 1UL;
    if (reload > SYSTICK_LOAD_RELOAD_MASK)
    {
        return false;
    }

    g_millisecond_tick = 0UL;
    g_tick_rate_hz = tick_rate_hz;
    STM32F1_SYSTICK->CTRL = 0UL;
    STM32F1_SYSTICK->LOAD = reload;
    STM32F1_SYSTICK->VAL = 0UL;
    STM32F1_SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE |
                            SYSTICK_CTRL_TICKINT |
                            SYSTICK_CTRL_ENABLE;
    return true;
}

uint32_t hr_hw_timer_millis(void)
{
    if (g_tick_rate_hz == 0UL)
    {
        return 0UL;
    }
    return (g_millisecond_tick * 1000UL) / g_tick_rate_hz;
}

void hr_hw_timer_delay_ms(uint32_t milliseconds)
{
    const uint32_t start = hr_hw_timer_millis();

    while ((uint32_t)(hr_hw_timer_millis() - start) < milliseconds)
    {
        stm32f1_cpu_wait_for_interrupt();
    }
}

void hr_hw_timer_tick_isr(void)
{
    g_millisecond_tick++;
}
