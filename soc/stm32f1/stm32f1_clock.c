#include "stm32f1.h"

#define CLOCK_STARTUP_TIMEOUT  0x00050000UL

static bool wait_for_mask(volatile uint32_t *reg,
                          uint32_t mask,
                          uint32_t expected)
{
    uint32_t timeout = CLOCK_STARTUP_TIMEOUT;

    while (((*reg) & mask) != expected)
    {
        if (timeout == 0UL)
        {
            return false;
        }
        timeout--;
    }

    return true;
}

bool stm32f1_clock_configure_hse_pll_72mhz(void)
{
    STM32F1_RCC->CR |= RCC_CR_HSEON;
    if (!wait_for_mask(&STM32F1_RCC->CR, RCC_CR_HSERDY, RCC_CR_HSERDY))
    {
        return false;
    }

    STM32F1_FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;

    STM32F1_RCC->CFGR = RCC_CFGR_HPRE_DIV1 |
                        RCC_CFGR_PPRE1_DIV2 |
                        RCC_CFGR_PPRE2_DIV1 |
                        RCC_CFGR_ADCPRE_DIV6 |
                        RCC_CFGR_PLLSRC_HSE |
                        RCC_CFGR_PLLMULL9;

    STM32F1_RCC->CR |= RCC_CR_PLLON;
    if (!wait_for_mask(&STM32F1_RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY))
    {
        return false;
    }

    STM32F1_RCC->CFGR = (STM32F1_RCC->CFGR & ~RCC_CFGR_SW_MASK) |
                        RCC_CFGR_SW_PLL;

    if (!wait_for_mask(&STM32F1_RCC->CFGR,
                       RCC_CFGR_SWS_MASK,
                       RCC_CFGR_SWS_PLL))
    {
        return false;
    }

    SystemCoreClock = 72000000UL;
    return true;
}

static uint32_t decode_ahb_prescaler(uint32_t cfgr)
{
    static const uint16_t divisors[16] =
    {
        1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U,
        2U, 4U, 8U, 16U, 64U, 128U, 256U, 512U
    };
    const uint32_t index = (cfgr >> 4U) & 0x0FUL;
    return (uint32_t)divisors[index];
}

static uint32_t decode_apb_prescaler(uint32_t encoded)
{
    static const uint8_t divisors[8] = {1U, 1U, 1U, 1U, 2U, 4U, 8U, 16U};
    return (uint32_t)divisors[encoded & 0x07UL];
}

uint32_t stm32f1_clock_get_hclk_hz(void)
{
    const uint32_t cfgr = STM32F1_RCC->CFGR;
    const uint32_t sws = cfgr & RCC_CFGR_SWS_MASK;
    uint32_t sysclk_hz = 8000000UL;

    if (sws == RCC_CFGR_SWS_PLL)
    {
        const uint32_t multiplier = ((cfgr & RCC_CFGR_PLLMULL_MASK) >> 18U) + 2UL;
        const uint32_t pll_input_hz = ((cfgr & RCC_CFGR_PLLSRC_HSE) != 0UL)
                                          ? 8000000UL
                                          : 4000000UL;
        sysclk_hz = pll_input_hz * multiplier;
    }

    return sysclk_hz / decode_ahb_prescaler(cfgr);
}

uint32_t stm32f1_clock_get_pclk1_hz(void)
{
    const uint32_t cfgr = STM32F1_RCC->CFGR;
    return stm32f1_clock_get_hclk_hz() /
           decode_apb_prescaler((cfgr >> 8U) & 0x07UL);
}

uint32_t stm32f1_clock_get_pclk2_hz(void)
{
    const uint32_t cfgr = STM32F1_RCC->CFGR;
    return stm32f1_clock_get_hclk_hz() /
           decode_apb_prescaler((cfgr >> 11U) & 0x07UL);
}
