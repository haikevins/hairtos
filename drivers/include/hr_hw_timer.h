
#ifndef HR_HW_TIMER_H
#define HR_HW_TIMER_H

#include <stdbool.h>
#include <stdint.h>

bool hr_hw_timer_init(uint32_t tick_rate_hz);
uint32_t hr_hw_timer_millis(void);
void hr_hw_timer_delay_ms(uint32_t milliseconds);
void hr_hw_timer_tick_isr(void);

#endif /* HR_HW_TIMER_H */
