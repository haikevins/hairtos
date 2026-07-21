#ifndef HAIRTOS_BOARD_PINS_H
#define HAIRTOS_BOARD_PINS_H

#include "hr_gpio.h"

#define BOARD_LED_PORT          HR_GPIO_PORT_C
#define BOARD_LED_PIN           13U
#define BOARD_LED_ACTIVE_LOW    1

#define BOARD_UART_TX_PORT      HR_GPIO_PORT_A
#define BOARD_UART_TX_PIN       9U
#define BOARD_UART_RX_PORT      HR_GPIO_PORT_A
#define BOARD_UART_RX_PIN       10U

#endif /* HAIRTOS_BOARD_PINS_H */
