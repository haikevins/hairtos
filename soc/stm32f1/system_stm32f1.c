#include "stm32f1.h"

extern const uint32_t g_pfnVectors[];

uint32_t SystemCoreClock = 8000000UL;

static void system_reset_clock_tree(void)
{
    STM32F1_RCC->CR |= RCC_CR_HSION;
    while ((STM32F1_RCC->CR & RCC_CR_HSIRDY) == 0UL)
    {
    }

    STM32F1_RCC->CFGR = 0UL;
    STM32F1_RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON | RCC_CR_PLLON);
    STM32F1_RCC->CIR = 0UL;
    STM32F1_FLASH->ACR = FLASH_ACR_LATENCY_0;
    SystemCoreClock = 8000000UL;
}

void SystemInit(void)
{
    system_reset_clock_tree();
    STM32F1_SCB->VTOR = (uint32_t)(uintptr_t)g_pfnVectors;

    (void)stm32f1_clock_configure_hse_pll_72mhz();
    SystemCoreClockUpdate();
}

void SystemCoreClockUpdate(void)
{
    SystemCoreClock = stm32f1_clock_get_hclk_hz();
}
