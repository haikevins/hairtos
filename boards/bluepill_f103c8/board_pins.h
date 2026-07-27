
#ifndef HAIRTOS_BOARD_PINS_H
#define HAIRTOS_BOARD_PINS_H

#include "hr_gpio_stm32f1.h"
#include "hr_uart_stm32f1.h"

#define BOARD_LED_PIN               HR_STM32F1_GPIO_PIN_C(13U)
#define BOARD_LED_ACTIVE_LOW        1

#define BOARD_UART_INSTANCE         HR_STM32F1_UART1
#define BOARD_UART_TX_PIN           HR_STM32F1_GPIO_PIN_A(9U)
#define BOARD_UART_RX_PIN           HR_STM32F1_GPIO_PIN_A(10U)

#define BOARD_BENCHMARK_MARKER_PIN  HR_STM32F1_GPIO_PIN_B(0U)

#endif /* HAIRTOS_BOARD_PINS_H */
