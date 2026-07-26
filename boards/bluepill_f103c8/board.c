#include "board.h"
#include "board_pins.h"
#include "hr_gpio.h"
#include "hr_hw_timer.h"
#include "hr_uart.h"
#include "stm32f1.h"

#define BOARD_UART_BAUD_RATE 115200UL

static void board_print_boot_banner(void)
{
    board_uart_write_line("");
    board_uart_write_line("HairRTOS platform boot");
    board_uart_write_line("Board: Blue Pill STM32F103C8T6");
    board_uart_write_string("Core clock: ");
    board_uart_write_u32(board_get_core_clock_hz());
    board_uart_write_line(" Hz");

    if (board_clock_is_72mhz())
    {
        board_uart_write_line("Clock source: HSE 8 MHz -> PLL x9");
    }
    else
    {
        board_uart_write_line("WARNING: HSE/PLL startup failed; using HSI fallback");
    }
}

void board_init(void)
{
    hr_gpio_enable_port_clock(BOARD_LED_PORT);
    hr_gpio_config_output_push_pull(BOARD_LED_PORT, BOARD_LED_PIN, HR_GPIO_SPEED_2MHZ);
    board_led_off();

    hr_uart_init(BOARD_UART_BAUD_RATE, stm32f1_clock_get_pclk2_hz());
    hr_hw_timer_init_1khz(SystemCoreClock);

    board_print_boot_banner();
}

void board_led_on(void)
{
#if BOARD_LED_ACTIVE_LOW
    hr_gpio_write(BOARD_LED_PORT, BOARD_LED_PIN, false);
#else
    hr_gpio_write(BOARD_LED_PORT, BOARD_LED_PIN, true);
#endif
}

void board_led_off(void)
{
#if BOARD_LED_ACTIVE_LOW
    hr_gpio_write(BOARD_LED_PORT, BOARD_LED_PIN, true);
#else
    hr_gpio_write(BOARD_LED_PORT, BOARD_LED_PIN, false);
#endif
}

void board_led_toggle(void)
{
    hr_gpio_toggle(BOARD_LED_PORT, BOARD_LED_PIN);
}

void board_uart_write_char(char character)
{
    hr_uart_write_char(character);
}

void board_uart_write_string(const char *text)
{
    hr_uart_write_string(text);
}

void board_uart_write_u32(uint32_t value)
{
    char digits[10];
    uint32_t count = 0UL;

    if (value == 0UL)
    {
        board_uart_write_char('0');
        return;
    }

    while ((value != 0UL) && (count < (uint32_t)sizeof(digits)))
    {
        digits[count] = (char)('0' + (value % 10UL));
        value /= 10UL;
        count++;
    }

    while (count != 0UL)
    {
        count--;
        board_uart_write_char(digits[count]);
    }
}

void board_uart_write_line(const char *text)
{
    board_uart_write_string(text);
    board_uart_write_string("\r\n");
}

uint32_t board_millis(void)
{
    return hr_hw_timer_millis();
}

void board_delay_ms(uint32_t milliseconds)
{
    hr_hw_timer_delay_ms(milliseconds);
}

void board_panic(void)
{
    stm32f1_cpu_disable_irq();
    board_led_on();

    for (;;)
    {
        stm32f1_cpu_breakpoint();
    }
}
