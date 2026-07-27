
#ifndef HR_GPIO_STM32F1_H
#define HR_GPIO_STM32F1_H

#include <stdint.h>

#include "hr_gpio.h"

#define HR_STM32F1_GPIO_PORT_A 0U
#define HR_STM32F1_GPIO_PORT_B 1U
#define HR_STM32F1_GPIO_PORT_C 2U

#define HR_STM32F1_GPIO_PIN(port_index, pin_index) \
    ((hr_gpio_pin_t)((((uint32_t)(port_index)) << 8U) | \
                     ((uint32_t)(pin_index) & 0x1FU)))
#define HR_STM32F1_GPIO_PIN_A(pin_index) \
    HR_STM32F1_GPIO_PIN(HR_STM32F1_GPIO_PORT_A, (pin_index))
#define HR_STM32F1_GPIO_PIN_B(pin_index) \
    HR_STM32F1_GPIO_PIN(HR_STM32F1_GPIO_PORT_B, (pin_index))
#define HR_STM32F1_GPIO_PIN_C(pin_index) \
    HR_STM32F1_GPIO_PIN(HR_STM32F1_GPIO_PORT_C, (pin_index))

#endif /* HR_GPIO_STM32F1_H */
