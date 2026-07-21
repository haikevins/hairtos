#include "hr_uart.h"
#include "hr_gpio.h"
#include "stm32f1.h"

#define UART1_TX_PIN 9UL
#define UART1_RX_PIN 10UL

void hr_uart_init(uint32_t baud_rate, uint32_t peripheral_clock_hz)
{
    if ((baud_rate == 0UL) || (peripheral_clock_hz == 0UL))
    {
        return;
    }

    STM32F1_RCC->APB2ENR |= RCC_APB2ENR_AFIOEN |
                            RCC_APB2ENR_IOPAEN |
                            RCC_APB2ENR_USART1EN;

    hr_gpio_config_alternate_push_pull(HR_GPIO_PORT_A,
                                       UART1_TX_PIN,
                                       HR_GPIO_SPEED_50MHZ);
    hr_gpio_config_input_floating(HR_GPIO_PORT_A, UART1_RX_PIN);

    STM32F1_USART1->CR1 = 0UL;
    STM32F1_USART1->CR2 = 0UL;
    STM32F1_USART1->CR3 = 0UL;
    STM32F1_USART1->BRR = (peripheral_clock_hz + (baud_rate / 2UL)) / baud_rate;
    STM32F1_USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
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
