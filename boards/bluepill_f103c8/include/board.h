#ifndef HAIRTOS_BOARD_H
#define HAIRTOS_BOARD_H

#include <stdbool.h>
#include <stdint.h>

void board_init(void);

void board_led_on(void);
void board_led_off(void);
void board_led_toggle(void);

void board_uart_write_char(char character);
void board_uart_write_string(const char *text);
void board_uart_write_u32(uint32_t value);
void board_uart_write_hex32(uint32_t value);
void board_uart_write_line(const char *text);

uint32_t board_millis(void);
void board_delay_ms(uint32_t milliseconds);

uint32_t board_get_core_clock_hz(void);
bool board_clock_is_72mhz(void);

void board_panic(void);

#endif /* HAIRTOS_BOARD_H */
