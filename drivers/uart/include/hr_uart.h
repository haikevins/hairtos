#ifndef HR_UART_H
#define HR_UART_H

#include <stdbool.h>
#include <stdint.h>

void hr_uart_init(uint32_t baud_rate, uint32_t peripheral_clock_hz);
void hr_uart_write_char(char character);
void hr_uart_write_string(const char *text);
bool hr_uart_try_read_char(char *character);

#endif /* HR_UART_H */
