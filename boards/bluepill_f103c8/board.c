
#include "board.h"
#include "board_pins.h"
#include "hr_gpio.h"
#include "hr_hw_timer.h"
#include "hr_uart.h"
#include "stm32f1.h"

#define BOARD_UART_BAUD_RATE 115200UL
#define BOARD_TIMER_RATE_HZ   1000UL

extern unsigned char __flash_start__;
extern unsigned char __flash_image_end__;
extern unsigned char __ram_start__;
extern unsigned char __static_ram_end__;

static void board_print_boot_banner(void)
{
    board_uart_write_line("");
    board_uart_write_line("hairtos platform boot");
    board_uart_write_string("Board: ");
    board_uart_write_line(board_get_name());
    board_uart_write_string("CPU: ");
    board_uart_write_line(board_get_cpu_name());
    board_uart_write_string("Core clock: ");
    board_uart_write_u32(board_get_core_clock_hz());
    board_uart_write_line(" Hz");

    if (board_clock_is_nominal())
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
    static const hr_gpio_config_t led_config =
    {
        .mode = HR_GPIO_MODE_OUTPUT_PUSH_PULL,
        .pull = HR_GPIO_PULL_NONE,
        .drive = HR_GPIO_DRIVE_LOW
    };
    const hr_uart_config_t uart_config =
    {
        .instance = BOARD_UART_INSTANCE,
        .baud_rate = BOARD_UART_BAUD_RATE,
        .tx_pin = BOARD_UART_TX_PIN,
        .rx_pin = BOARD_UART_RX_PIN
    };

    if (!hr_gpio_configure(BOARD_LED_PIN, &led_config))
    {
        board_panic();
    }
    board_led_off();

    if (!hr_uart_init(&uart_config))
    {
        board_panic();
    }
    if (!hr_hw_timer_init(BOARD_TIMER_RATE_HZ))
    {
        board_panic();
    }

    board_print_boot_banner();
}

const char *board_get_name(void)
{
    return "Blue Pill STM32F103C8T6";
}

const char *board_get_cpu_name(void)
{
    return "STM32F103C8T6 ARM Cortex-M3";
}

void board_led_on(void)
{
#if BOARD_LED_ACTIVE_LOW
    hr_gpio_write(BOARD_LED_PIN, false);
#else
    hr_gpio_write(BOARD_LED_PIN, true);
#endif
}

void board_led_off(void)
{
#if BOARD_LED_ACTIVE_LOW
    hr_gpio_write(BOARD_LED_PIN, true);
#else
    hr_gpio_write(BOARD_LED_PIN, false);
#endif
}

void board_led_toggle(void)
{
    hr_gpio_toggle(BOARD_LED_PIN);
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

void board_uart_write_hex32(uint32_t value)
{
    static const char digits[] = "0123456789ABCDEF";
    int shift;

    board_uart_write_string("0x");
    for (shift = 28; shift >= 0; shift -= 4)
    {
        board_uart_write_char(digits[(value >> (uint32_t)shift) & 0x0FU]);
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

uint32_t board_get_flash_image_bytes(void)
{
    return (uint32_t)((uintptr_t)&__flash_image_end__ -
                      (uintptr_t)&__flash_start__);
}

uint32_t board_get_static_ram_bytes(void)
{
    return (uint32_t)((uintptr_t)&__static_ram_end__ -
                      (uintptr_t)&__ram_start__);
}

bool board_benchmark_marker_init(void)
{
    static const hr_gpio_config_t marker_config =
    {
        .mode = HR_GPIO_MODE_OUTPUT_PUSH_PULL,
        .pull = HR_GPIO_PULL_NONE,
        .drive = HR_GPIO_DRIVE_HIGH
    };

    if (!hr_gpio_configure(BOARD_BENCHMARK_MARKER_PIN, &marker_config))
    {
        return false;
    }
    hr_gpio_write(BOARD_BENCHMARK_MARKER_PIN, false);
    return true;
}

void board_benchmark_marker_begin(void)
{
    hr_gpio_write(BOARD_BENCHMARK_MARKER_PIN, true);
}

void board_benchmark_marker_end(void)
{
    hr_gpio_write(BOARD_BENCHMARK_MARKER_PIN, false);
}

const char *board_benchmark_marker_description(void)
{
    return "PB0 active-high around switch/wake samples";
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
