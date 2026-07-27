
#include "hr_uart.h"
#include "hr_uart_stm32f1.h"
#include "hr_gpio.h"
#include "stm32f1.h"

bool hr_uart_init(const hr_uart_config_t *config)
{
    static const hr_gpio_config_t tx_config =
    {
        .mode = HR_GPIO_MODE_ALTERNATE_PUSH_PULL,
        .pull = HR_GPIO_PULL_NONE,
        .drive = HR_GPIO_DRIVE_HIGH
    };
    static const hr_gpio_config_t rx_config =
    {
        .mode = HR_GPIO_MODE_INPUT,
        .pull = HR_GPIO_PULL_NONE,
        .drive = HR_GPIO_DRIVE_LOW
    };
    uint32_t peripheral_clock_hz;

    if ((config == (const hr_uart_config_t *)0) ||
        (config->instance != HR_STM32F1_UART1) ||
        (config->baud_rate == 0UL))
    {
        return false;
    }

    STM32F1_RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN;

    if (!hr_gpio_configure(config->tx_pin, &tx_config) ||
        !hr_gpio_configure(config->rx_pin, &rx_config))
    {
        return false;
    }

    peripheral_clock_hz = stm32f1_clock_get_pclk2_hz();
    if (peripheral_clock_hz == 0UL)
    {
        return false;
    }

    STM32F1_USART1->CR1 = 0UL;
    STM32F1_USART1->CR2 = 0UL;
    STM32F1_USART1->CR3 = 0UL;
    STM32F1_USART1->BRR =
        (peripheral_clock_hz + (config->baud_rate / 2UL)) /
        config->baud_rate;
    STM32F1_USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
    return true;
}

void hr_uart_write_char(char character)
{
    while ((STM32F1_USART1->SR & USART_SR_TXE) == 0UL)
    {
    }

    STM32F1_USART1->DR = (uint32_t)(uint8_t)character;
}

void hr_uart_write_string(const char *text)
{
    if (text == (const char *)0)
    {
        return;
    }

    while (*text != '\0')
    {
        hr_uart_write_char(*text);
        text++;
    }
}

bool hr_uart_try_read_char(char *character)
{
    if ((character == (char *)0) ||
        ((STM32F1_USART1->SR & USART_SR_RXNE) == 0UL))
    {
        return false;
    }

    *character = (char)(uint8_t)STM32F1_USART1->DR;
    return true;
}
