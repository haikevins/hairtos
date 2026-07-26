#include "hr_gpio.h"
#include "stm32f1.h"

#define GPIO_PIN_COUNT 16UL

static stm32f1_gpio_t *gpio_from_port(hr_gpio_port_t port)
{
    switch (port)
    {
        case HR_GPIO_PORT_A:
            return STM32F1_GPIOA;
        case HR_GPIO_PORT_B:
            return STM32F1_GPIOB;
        case HR_GPIO_PORT_C:
            return STM32F1_GPIOC;
        default:
            return (stm32f1_gpio_t *)0;
    }
}

static void gpio_configure_nibble(stm32f1_gpio_t *gpio,
                                  uint32_t pin,
                                  uint32_t nibble)
{
    volatile uint32_t *config_register;
    uint32_t shift;
    uint32_t value;

    if ((gpio == (stm32f1_gpio_t *)0) || (pin >= GPIO_PIN_COUNT))
    {
        return;
    }

    if (pin < 8UL)
    {
        config_register = &gpio->CRL;
        shift = pin * 4UL;
    }
    else
    {
        config_register = &gpio->CRH;
        shift = (pin - 8UL) * 4UL;
    }

    value = *config_register;
    value &= ~(0x0FUL << shift);
    value |= (nibble & 0x0FUL) << shift;
    *config_register = value;
}

void hr_gpio_enable_port_clock(hr_gpio_port_t port)
{
    switch (port)
    {
        case HR_GPIO_PORT_A:
            STM32F1_RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
            break;
        case HR_GPIO_PORT_B:
            STM32F1_RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
            break;
        case HR_GPIO_PORT_C:
            STM32F1_RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
            break;
        default:
            break;
    }
}

void hr_gpio_config_output_push_pull(hr_gpio_port_t port,
                                     uint32_t pin,
                                     hr_gpio_speed_t speed)
{
    gpio_configure_nibble(gpio_from_port(port), pin, (uint32_t)speed);
}

void hr_gpio_config_alternate_push_pull(hr_gpio_port_t port,
                                        uint32_t pin,
                                        hr_gpio_speed_t speed)
{
    const uint32_t config = (2UL << 2U) | (uint32_t)speed;
    gpio_configure_nibble(gpio_from_port(port), pin, config);
}

void hr_gpio_config_input_floating(hr_gpio_port_t port, uint32_t pin)
{
    gpio_configure_nibble(gpio_from_port(port), pin, 4UL);
}

void hr_gpio_write(hr_gpio_port_t port, uint32_t pin, bool high)
{
    stm32f1_gpio_t *const gpio = gpio_from_port(port);

    if ((gpio == (stm32f1_gpio_t *)0) || (pin >= GPIO_PIN_COUNT))
    {
        return;
    }

    if (high)
    {
        gpio->BSRR = 1UL << pin;
    }
    else
    {
        gpio->BRR = 1UL << pin;
    }
}

bool hr_gpio_read(hr_gpio_port_t port, uint32_t pin)
{
    stm32f1_gpio_t *const gpio = gpio_from_port(port);

    if ((gpio == (stm32f1_gpio_t *)0) || (pin >= GPIO_PIN_COUNT))
    {
        return false;
    }

    return (gpio->IDR & (1UL << pin)) != 0UL;
}

void hr_gpio_toggle(hr_gpio_port_t port, uint32_t pin)
{
    stm32f1_gpio_t *const gpio = gpio_from_port(port);

    if ((gpio == (stm32f1_gpio_t *)0) || (pin >= GPIO_PIN_COUNT))
    {
        return;
    }

    if ((gpio->ODR & (1UL << pin)) != 0UL)
    {
        gpio->BRR = 1UL << pin;
    }
    else
    {
        gpio->BSRR = 1UL << pin;
    }
}
