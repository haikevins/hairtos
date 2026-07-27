
#ifndef HR_UART_H
#define HR_UART_H

#include <stdbool.h>
#include <stdint.h>

#include "hr_gpio.h"

/* Opaque target-defined peripheral identifier. */
typedef uint32_t hr_uart_instance_t;

typedef struct
{
    hr_uart_instance_t instance;
    uint32_t baud_rate;
    hr_gpio_pin_t tx_pin;
    hr_gpio_pin_t rx_pin;
} hr_uart_config_t;

bool hr_uart_init(const hr_uart_config_t *config);
void hr_uart_write_char(char character);
void hr_uart_write_string(const char *text);
bool hr_uart_try_read_char(char *character);

#endif /* HR_UART_H */
