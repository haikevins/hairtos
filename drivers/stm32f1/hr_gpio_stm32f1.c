
#include "hr_gpio.h"
#include "hr_gpio_stm32f1.h"
#include "stm32f1.h"

#define GPIO_PIN_COUNT 16UL
#define GPIO_PORT_SHIFT 8U
#define GPIO_PORT_MASK  0xFFUL
#define GPIO_PIN_MASK   0x1FUL

static uint32_t gpio_port_index(hr_gpio_pin_t pin)
{
    return ((uint32_t)pin >> GPIO_PORT_SHIFT) & GPIO_PORT_MASK;
}

static uint32_t gpio_pin_index(hr_gpio_pin_t pin)
{
    return (uint32_t)pin & GPIO_PIN_MASK;
}

static stm32f1_gpio_t *gpio_from_pin(hr_gpio_pin_t pin)
{
    switch (gpio_port_index(pin))
    {
        case HR_STM32F1_GPIO_PORT_A:
            return STM32F1_GPIOA;
        case HR_STM32F1_GPIO_PORT_B:
            return STM32F1_GPIOB;
        case HR_STM32F1_GPIO_PORT_C:
            return STM32F1_GPIOC;
        default:
            return (stm32f1_gpio_t *)0;
    }
}

static void gpio_enable_port_clock(hr_gpio_pin_t pin)
{
    switch (gpio_port_index(pin))
    {
        case HR_STM32F1_GPIO_PORT_A:
            STM32F1_RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
            break;
        case HR_STM32F1_GPIO_PORT_B:
            STM32F1_RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
            break;
        case HR_STM32F1_GPIO_PORT_C:
            STM32F1_RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
            break;
        default:
            break;
    }
}

static uint32_t gpio_mode_bits(hr_gpio_drive_t drive)
{
    switch (drive)
    {
        case HR_GPIO_DRIVE_LOW:
            return 2UL; /* STM32F1 output mode 2 MHz. */
        case HR_GPIO_DRIVE_MEDIUM:
            return 1UL; /* STM32F1 output mode 10 MHz. */
        case HR_GPIO_DRIVE_HIGH:
        case HR_GPIO_DRIVE_VERY_HIGH:
            return 3UL; /* STM32F1 output mode 50 MHz. */
        default:
            return 2UL;
    }
}

static bool gpio_build_nibble(const hr_gpio_config_t *config,
                              uint32_t *nibble)
{
    uint32_t value;

    if ((config == (const hr_gpio_config_t *)0) ||
        (nibble == (uint32_t *)0))
    {
        return false;
    }

    switch (config->mode)
    {
        case HR_GPIO_MODE_INPUT:
            if (config->pull == HR_GPIO_PULL_NONE)
            {
                value = 0x4UL; /* Floating input. */
            }
            else
            {
                value = 0x8UL; /* Pull-up or pull-down input. */
            }
            break;
        case HR_GPIO_MODE_OUTPUT_PUSH_PULL:
            value = gpio_mode_bits(config->drive);
            break;
        case HR_GPIO_MODE_OUTPUT_OPEN_DRAIN:
            value = gpio_mode_bits(config->drive) | 0x4UL;
            break;
        case HR_GPIO_MODE_ALTERNATE_PUSH_PULL:
            value = gpio_mode_bits(config->drive) | 0x8UL;
            break;
        case HR_GPIO_MODE_ALTERNATE_OPEN_DRAIN:
            value = gpio_mode_bits(config->drive) | 0xCUL;
            break;
        default:
            return false;
    }

    *nibble = value;
    return true;
}

bool hr_gpio_configure(hr_gpio_pin_t pin, const hr_gpio_config_t *config)
{
    stm32f1_gpio_t *const gpio = gpio_from_pin(pin);
    const uint32_t pin_index = gpio_pin_index(pin);
    volatile uint32_t *config_register;
    uint32_t shift;
    uint32_t nibble;
    uint32_t value;

    if ((gpio == (stm32f1_gpio_t *)0) ||
        (pin_index >= GPIO_PIN_COUNT) ||
        !gpio_build_nibble(config, &nibble))
    {
        return false;
    }

    gpio_enable_port_clock(pin);

    if (pin_index < 8UL)
    {
        config_register = &gpio->CRL;
        shift = pin_index * 4UL;
    }
    else
    {
        config_register = &gpio->CRH;
        shift = (pin_index - 8UL) * 4UL;
    }

    value = *config_register;
    value &= ~(0x0FUL << shift);
    value |= (nibble & 0x0FUL) << shift;
    *config_register = value;

    if ((config->mode == HR_GPIO_MODE_INPUT) &&
        (config->pull != HR_GPIO_PULL_NONE))
    {
        hr_gpio_write(pin, config->pull == HR_GPIO_PULL_UP);
    }

    return true;
}

void hr_gpio_write(hr_gpio_pin_t pin, bool high)
{
    stm32f1_gpio_t *const gpio = gpio_from_pin(pin);
    const uint32_t pin_index = gpio_pin_index(pin);

    if ((gpio == (stm32f1_gpio_t *)0) || (pin_index >= GPIO_PIN_COUNT))
    {
        return;
    }

    if (high)
    {
        gpio->BSRR = 1UL << pin_index;
    }
    else
    {
        gpio->BRR = 1UL << pin_index;
    }
}

bool hr_gpio_read(hr_gpio_pin_t pin)
{
    stm32f1_gpio_t *const gpio = gpio_from_pin(pin);
    const uint32_t pin_index = gpio_pin_index(pin);

    if ((gpio == (stm32f1_gpio_t *)0) || (pin_index >= GPIO_PIN_COUNT))
    {
        return false;
    }

    return (gpio->IDR & (1UL << pin_index)) != 0UL;
}

void hr_gpio_toggle(hr_gpio_pin_t pin)
{
    stm32f1_gpio_t *const gpio = gpio_from_pin(pin);
    const uint32_t pin_index = gpio_pin_index(pin);

    if ((gpio == (stm32f1_gpio_t *)0) || (pin_index >= GPIO_PIN_COUNT))
    {
        return;
    }

    if ((gpio->ODR & (1UL << pin_index)) != 0UL)
    {
        gpio->BRR = 1UL << pin_index;
    }
    else
    {
        gpio->BSRR = 1UL << pin_index;
    }
}
