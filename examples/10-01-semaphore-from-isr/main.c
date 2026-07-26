#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_semaphore.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_time.h"
#include "hr_port.h"
#include "stm32f1.h"

#define PHASE10_SEM_WAITER_PRIORITY       1U
#define PHASE10_SEM_TRIGGER_PRIORITY      3U
#define PHASE10_SEM_STACK_WORDS           192U
#define PHASE10_SEM_PERIOD_TICKS          500U

#define STM32F1_EXTI_BASE                 0x40010400UL
#define STM32F1_NVIC_ISER0                0xE000E100UL
#define STM32F1_EXTI_IMR                  STM32F1_REG32(STM32F1_EXTI_BASE + 0x00UL)
#define STM32F1_EXTI_SWIER                STM32F1_REG32(STM32F1_EXTI_BASE + 0x10UL)
#define STM32F1_EXTI_PR                   STM32F1_REG32(STM32F1_EXTI_BASE + 0x14UL)
#define STM32F1_NVIC_ISER0_REG            STM32F1_REG32(STM32F1_NVIC_ISER0)
#define STM32F1_EXTI_LINE0                (1UL << 0U)
#define STM32F1_EXTI0_IRQ_BIT             (1UL << 6U)

static hr_semaphore_t g_irq_semaphore;
static hr_task_t g_waiter_task;
static hr_task_t g_trigger_task;
static hr_stack_t g_waiter_stack[PHASE10_SEM_STACK_WORDS];
static hr_stack_t g_trigger_stack[PHASE10_SEM_STACK_WORDS];
static volatile uint32_t g_irq_count;
static volatile hr_status_t g_irq_status = HR_OK;

static void phase10_sem_configure_software_irq(void)
{
    STM32F1_EXTI_PR = STM32F1_EXTI_LINE0;
    STM32F1_EXTI_IMR |= STM32F1_EXTI_LINE0;
    STM32F1_NVIC_ISER0_REG = STM32F1_EXTI0_IRQ_BIT;
}

static void phase10_sem_trigger_irq(void)
{
    STM32F1_EXTI_SWIER = STM32F1_EXTI_LINE0;
}

void EXTI0_IRQHandler(void)
{
    bool higher_priority_task_woken = false;

    if ((STM32F1_EXTI_PR & STM32F1_EXTI_LINE0) != 0U)
    {
        STM32F1_EXTI_PR = STM32F1_EXTI_LINE0;
        g_irq_count++;
        g_irq_status = hr_semaphore_give_from_isr(
            &g_irq_semaphore,
            &higher_priority_task_woken);
        hr_port_yield_from_isr(higher_priority_task_woken);
    }
}

static void waiter_task(void *argument)
{
    uint32_t wake_count = 0U;

    (void)argument;

    for (;;)
    {
        hr_status_t status = hr_semaphore_take(&g_irq_semaphore,
                                               HR_WAIT_FOREVER);
        if ((status != HR_OK) || (g_irq_status != HR_OK) ||
            (hr_task_current() != &g_waiter_task) ||
            !hr_port_thread_uses_psp())
        {
            board_uart_write_line("ERROR: semaphore ISR handoff failed.");
            board_panic();
        }

        wake_count++;
        board_led_toggle();
        board_uart_write_string("waiter wake=");
        board_uart_write_u32(wake_count);
        board_uart_write_string(" irq_count=");
        board_uart_write_u32(g_irq_count);
        board_uart_write_string(" tick=");
        board_uart_write_u32(hr_time_now());
        board_uart_write_line("");
    }
}

static void trigger_task(void *argument)
{
    hr_tick_t next_release = hr_time_now();

    (void)argument;

    for (;;)
    {
        if (hr_task_delay_until(&next_release,
                                PHASE10_SEM_PERIOD_TICKS) != HR_OK)
        {
            board_uart_write_line("ERROR: trigger delay failed.");
            board_panic();
        }

        phase10_sem_trigger_irq();
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("HairRTOS Phase 10.1");
    board_uart_write_line("EXTI0 software interrupt gives a binary semaphore.");
    board_uart_write_line("The higher-priority waiter preempts after ISR return.");

    status = hr_semaphore_create_binary(&g_irq_semaphore, false);
    if (status != HR_OK)
    {
        board_uart_write_line("Semaphore creation failed.");
        board_panic();
    }

    phase10_sem_configure_software_irq();

    status = hr_kernel_init();
    if (status != HR_OK)
    {
        board_uart_write_line("Kernel initialization failed.");
        board_panic();
    }

    if ((hr_task_create_static(&g_waiter_task,
                               "sem-waiter",
                               waiter_task,
                               NULL,
                               g_waiter_stack,
                               PHASE10_SEM_STACK_WORDS,
                               PHASE10_SEM_WAITER_PRIORITY) != HR_OK) ||
        (hr_task_create_static(&g_trigger_task,
                               "irq-trigger",
                               trigger_task,
                               NULL,
                               g_trigger_stack,
                               PHASE10_SEM_STACK_WORDS,
                               PHASE10_SEM_TRIGGER_PRIORITY) != HR_OK) ||
        (hr_task_start(&g_trigger_task) != HR_OK) ||
        (hr_task_start(&g_waiter_task) != HR_OK))
    {
        board_uart_write_line("Phase 10.1 task setup failed.");
        board_panic();
    }

    board_uart_write_line("Starting Phase 10.1 scheduler through SVC...");
    status = hr_kernel_start();
    board_uart_write_string("ERROR: hr_kernel_start returned status=");
    board_uart_write_u32((uint32_t)status);
    board_uart_write_line("");
    board_panic();
    return (int)status;
}
