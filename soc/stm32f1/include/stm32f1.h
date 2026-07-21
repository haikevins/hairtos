#ifndef STM32F1_H
#define STM32F1_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define STM32F1_PERIPH_BASE              0x40000000UL
#define STM32F1_APB1_BASE                STM32F1_PERIPH_BASE
#define STM32F1_APB2_BASE                0x40010000UL
#define STM32F1_AHB_BASE                 0x40018000UL

#define STM32F1_AFIO_BASE                (STM32F1_APB2_BASE + 0x0000UL)
#define STM32F1_GPIOA_BASE               (STM32F1_APB2_BASE + 0x0800UL)
#define STM32F1_GPIOB_BASE               (STM32F1_APB2_BASE + 0x0C00UL)
#define STM32F1_GPIOC_BASE               (STM32F1_APB2_BASE + 0x1000UL)
#define STM32F1_USART1_BASE              (STM32F1_APB2_BASE + 0x3800UL)
#define STM32F1_RCC_BASE                 (STM32F1_AHB_BASE  + 0x9000UL)
#define STM32F1_FLASH_INTERFACE_BASE     0x40022000UL

#define STM32F1_SYSTICK_BASE             0xE000E010UL
#define STM32F1_SCB_BASE                 0xE000ED00UL

#define STM32F1_REG32(address)           (*(volatile uint32_t *)(uintptr_t)(address))

typedef struct
{
    volatile uint32_t EVCR;
    volatile uint32_t MAPR;
    volatile uint32_t EXTICR[4];
    volatile uint32_t MAPR2;
} stm32f1_afio_t;

typedef struct
{
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} stm32f1_gpio_t;

typedef struct
{
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} stm32f1_usart_t;

typedef struct
{
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} stm32f1_rcc_t;

typedef struct
{
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t AR;
    volatile uint32_t RESERVED;
    volatile uint32_t OBR;
    volatile uint32_t WRPR;
} stm32f1_flash_t;

typedef struct
{
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} stm32f1_systick_t;

typedef struct
{
    volatile uint32_t CPUID;
    volatile uint32_t ICSR;
    volatile uint32_t VTOR;
    volatile uint32_t AIRCR;
    volatile uint32_t SCR;
    volatile uint32_t CCR;
    volatile uint8_t SHP[12];
    volatile uint32_t SHCSR;
} stm32f1_scb_t;

#define STM32F1_AFIO                     ((stm32f1_afio_t *)STM32F1_AFIO_BASE)
#define STM32F1_GPIOA                    ((stm32f1_gpio_t *)STM32F1_GPIOA_BASE)
#define STM32F1_GPIOB                    ((stm32f1_gpio_t *)STM32F1_GPIOB_BASE)
#define STM32F1_GPIOC                    ((stm32f1_gpio_t *)STM32F1_GPIOC_BASE)
#define STM32F1_USART1                   ((stm32f1_usart_t *)STM32F1_USART1_BASE)
#define STM32F1_RCC                      ((stm32f1_rcc_t *)STM32F1_RCC_BASE)
#define STM32F1_FLASH                    ((stm32f1_flash_t *)STM32F1_FLASH_INTERFACE_BASE)
#define STM32F1_SYSTICK                  ((stm32f1_systick_t *)STM32F1_SYSTICK_BASE)
#define STM32F1_SCB                      ((stm32f1_scb_t *)STM32F1_SCB_BASE)

/* RCC_CR */
#define RCC_CR_HSION                     (1UL << 0)
#define RCC_CR_HSIRDY                    (1UL << 1)
#define RCC_CR_HSEON                     (1UL << 16)
#define RCC_CR_HSERDY                    (1UL << 17)
#define RCC_CR_CSSON                     (1UL << 19)
#define RCC_CR_PLLON                     (1UL << 24)
#define RCC_CR_PLLRDY                    (1UL << 25)

/* RCC_CFGR */
#define RCC_CFGR_SW_MASK                 (3UL << 0)
#define RCC_CFGR_SW_HSI                  (0UL << 0)
#define RCC_CFGR_SW_PLL                  (2UL << 0)
#define RCC_CFGR_SWS_MASK                (3UL << 2)
#define RCC_CFGR_SWS_HSI                 (0UL << 2)
#define RCC_CFGR_SWS_PLL                 (2UL << 2)
#define RCC_CFGR_HPRE_DIV1               (0UL << 4)
#define RCC_CFGR_PPRE1_DIV2              (4UL << 8)
#define RCC_CFGR_PPRE2_DIV1              (0UL << 11)
#define RCC_CFGR_ADCPRE_DIV6             (2UL << 14)
#define RCC_CFGR_PLLSRC_HSE              (1UL << 16)
#define RCC_CFGR_PLLMULL9                 (7UL << 18)
#define RCC_CFGR_PLLMULL_MASK             (15UL << 18)

/* RCC_APB2ENR */
#define RCC_APB2ENR_AFIOEN               (1UL << 0)
#define RCC_APB2ENR_IOPAEN               (1UL << 2)
#define RCC_APB2ENR_IOPBEN               (1UL << 3)
#define RCC_APB2ENR_IOPCEN               (1UL << 4)
#define RCC_APB2ENR_USART1EN             (1UL << 14)

/* FLASH_ACR */
#define FLASH_ACR_LATENCY_MASK            (7UL << 0)
#define FLASH_ACR_LATENCY_0              (0UL << 0)
#define FLASH_ACR_LATENCY_2              (2UL << 0)
#define FLASH_ACR_PRFTBE                 (1UL << 4)

/* USART */
#define USART_SR_RXNE                    (1UL << 5)
#define USART_SR_TC                      (1UL << 6)
#define USART_SR_TXE                     (1UL << 7)
#define USART_CR1_RE                     (1UL << 2)
#define USART_CR1_TE                     (1UL << 3)
#define USART_CR1_UE                     (1UL << 13)

/* SysTick */
#define SYSTICK_CTRL_ENABLE              (1UL << 0)
#define SYSTICK_CTRL_TICKINT             (1UL << 1)
#define SYSTICK_CTRL_CLKSOURCE           (1UL << 2)
#define SYSTICK_LOAD_RELOAD_MASK         0x00FFFFFFUL

extern uint32_t SystemCoreClock;

void SystemInit(void);
void SystemCoreClockUpdate(void);

bool stm32f1_clock_configure_hse_pll_72mhz(void);
uint32_t stm32f1_clock_get_hclk_hz(void);
uint32_t stm32f1_clock_get_pclk1_hz(void);
uint32_t stm32f1_clock_get_pclk2_hz(void);

static inline void stm32f1_cpu_disable_irq(void)
{
    __asm volatile ("cpsid i" ::: "memory");
}

static inline void stm32f1_cpu_enable_irq(void)
{
    __asm volatile ("cpsie i" ::: "memory");
}

static inline void stm32f1_cpu_wait_for_interrupt(void)
{
    __asm volatile ("wfi" ::: "memory");
}

static inline void stm32f1_cpu_breakpoint(void)
{
    __asm volatile ("bkpt #0");
}

#endif /* STM32F1_H */
