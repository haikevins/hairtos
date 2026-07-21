#ifndef HR_HW_TIMER_H
#define HR_HW_TIMER_H

#include <stdint.h>

void hr_hw_timer_init_1khz(uint32_t core_clock_hz);
uint32_t hr_hw_timer_millis(void);
void hr_hw_timer_delay_ms(uint32_t milliseconds);

#endif /* HR_HW_TIMER_H */
