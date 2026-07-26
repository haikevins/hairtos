#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "haievent/haievent.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_time.h"
#include "hairtos/hr_context.h"
#include "stm32f1.h"

#define AO_PRIORITY       2U
#define TRIGGER_PRIORITY  4U
#define STACK_WORDS       224U
#define QUEUE_LENGTH      4U

#define STM32F1_EXTI_BASE       0x40010400UL
#define STM32F1_NVIC_ISER0      0xE000E100UL
#define STM32F1_EXTI_IMR        STM32F1_REG32(STM32F1_EXTI_BASE + 0x00UL)
#define STM32F1_EXTI_SWIER      STM32F1_REG32(STM32F1_EXTI_BASE + 0x10UL)
#define STM32F1_EXTI_PR         STM32F1_REG32(STM32F1_EXTI_BASE + 0x14UL)
#define STM32F1_NVIC_ISER0_REG  STM32F1_REG32(STM32F1_NVIC_ISER0)
#define STM32F1_EXTI_LINE0      (1UL << 0U)
#define STM32F1_EXTI0_IRQ_BIT   (1UL << 6U)

enum { SIGNAL_IRQ_SAMPLE = HE_SIG_USER };

static he_active_t g_receiver;
static he_event_t *g_receiver_queue[QUEUE_LENGTH];
static hr_stack_t g_receiver_stack[STACK_WORDS];
static hr_task_t g_trigger;
static hr_stack_t g_trigger_stack[STACK_WORDS];
static he_event_t g_irq_event;
static volatile uint32_t g_irq_count;
static volatile hr_status_t g_post_status = HR_OK;

static void configure_software_irq(void)
{
    STM32F1_EXTI_PR = STM32F1_EXTI_LINE0;
    STM32F1_EXTI_IMR |= STM32F1_EXTI_LINE0;
    STM32F1_NVIC_ISER0_REG = STM32F1_EXTI0_IRQ_BIT;
}

void EXTI0_IRQHandler(void)
{
    bool higher_priority_task_woken = false;

    if ((STM32F1_EXTI_PR & STM32F1_EXTI_LINE0) != 0U)
    {
        STM32F1_EXTI_PR = STM32F1_EXTI_LINE0;
        g_irq_count++;
        g_post_status = he_active_post_from_isr(&g_receiver,
                                                &g_irq_event,
                                                &higher_priority_task_woken);
        hr_yield_from_isr(higher_priority_task_woken);
    }
}

static he_state_result_t receiver_state(he_state_machine_t *machine,
                                        const he_event_t *event)
{
    (void)machine;

    if (event->signal == HE_SIG_ENTRY)
    {
        board_uart_write_line("receiver AO: blocked on event queue");
        return HE_STATE_HANDLED;
    }

    if (event->signal == SIGNAL_IRQ_SAMPLE)
    {
        if (g_post_status != HR_OK)
        {
            board_panic();
        }
        board_led_toggle();
        board_uart_write_string("receiver AO: ISR event count=");
        board_uart_write_u32(g_irq_count);
        board_uart_write_string(" tick=");
        board_uart_write_u32(hr_time_now());
        board_uart_write_line("");
        return HE_STATE_HANDLED;
    }

    return HE_STATE_IGNORED;
}

static void trigger_task(void *argument)
{
    (void)argument;

    for (;;)
    {
        (void)hr_task_delay(500U);
        STM32F1_EXTI_SWIER = STM32F1_EXTI_LINE0;
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("hairtos Phase 13.1 - ISR event post");
    board_uart_write_line("EXTI0 posts a static event; the AO dispatches after ISR return.");

    (void)he_event_init_static(&g_irq_event,
                               SIGNAL_IRQ_SAMPLE,
                               sizeof(g_irq_event));
    configure_software_irq();

    if (hr_kernel_init() != HR_OK)
    {
        board_panic();
    }

    if ((he_active_create_static(&g_receiver,
                                 "irq-receiver-AO",
                                 receiver_state,
                                 NULL,
                                 g_receiver_queue,
                                 QUEUE_LENGTH,
                                 g_receiver_stack,
                                 STACK_WORDS,
                                 AO_PRIORITY) != HR_OK) ||
        (hr_task_create_static(&g_trigger,
                               "irq-trigger",
                               trigger_task,
                               NULL,
                               g_trigger_stack,
                               STACK_WORDS,
                               TRIGGER_PRIORITY) != HR_OK) ||
        (hr_task_start(&g_trigger) != HR_OK))
    {
        board_panic();
    }

    status = hr_kernel_start();
    board_panic();
    return (int)status;
}
