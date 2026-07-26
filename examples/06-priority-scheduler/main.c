#include <stdint.h>

#include "board.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hr_port.h"

#define PHASE6_HIGH_PRIORITY  1U
#define PHASE6_LOW_PRIORITY   5U
#define PHASE6_STACK_WORDS    160U
#define PHASE6_DELAY_MS       250U

static hr_task_t g_low_task;
static hr_task_t g_high_task_a;
static hr_task_t g_high_task_b;

static hr_stack_t g_low_stack[PHASE6_STACK_WORDS];
static hr_stack_t g_high_stack_a[PHASE6_STACK_WORDS];
static hr_stack_t g_high_stack_b[PHASE6_STACK_WORDS];

static void phase6_verify_current(const hr_task_t *expected, const char *label)
{
    if (hr_task_current() != expected)
    {
        board_uart_write_string("ERROR: scheduler selected wrong task in ");
        board_uart_write_line(label);
        board_panic();
    }

    if (!hr_port_thread_uses_psp())
    {
        board_uart_write_line("ERROR: scheduled task is not using PSP.");
        board_panic();
    }
}

static void low_priority_task(void *argument)
{
    (void)argument;

    /*
     * This task is READY, but it must never execute while either priority-1
     * task remains READY. Reaching this function violates fixed-priority policy.
     */
    board_uart_write_line("ERROR: low-priority task ran while high tasks were READY.");
    board_panic();
}

static void high_priority_task_a(void *argument)
{
    uint32_t local_counter = 0U;

    (void)argument;

    for (;;)
    {
        phase6_verify_current(&g_high_task_a, "high task A");
        local_counter++;

        board_led_toggle();
        board_uart_write_string("selected=high-A priority=");
        board_uart_write_u32((uint32_t)hr_task_get_effective_priority(&g_high_task_a));
        board_uart_write_string(" counter=");
        board_uart_write_u32(local_counter);
        board_uart_write_line(" -> yield to equal-priority peer");

        board_delay_ms(PHASE6_DELAY_MS);
        hr_task_yield();
    }
}

static void high_priority_task_b(void *argument)
{
    uint32_t local_counter = 1000U;

    (void)argument;

    for (;;)
    {
        phase6_verify_current(&g_high_task_b, "high task B");
        local_counter += 10U;

        board_uart_write_string("selected=high-B priority=");
        board_uart_write_u32((uint32_t)hr_task_get_effective_priority(&g_high_task_b));
        board_uart_write_string(" counter=");
        board_uart_write_u32(local_counter);
        board_uart_write_line(" -> yield to equal-priority peer");

        board_delay_ms(PHASE6_DELAY_MS);
        hr_task_yield();
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("HairRTOS Phase 6");
    board_uart_write_line("Fixed-priority scheduler: smaller number means higher priority.");
    board_uart_write_line("Low task is registered first but must never run while high tasks are READY.");

    status = hr_kernel_init();
    if (status != HR_OK)
    {
        board_uart_write_line("Kernel initialization failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_low_task,
                                   "low",
                                   low_priority_task,
                                   NULL,
                                   g_low_stack,
                                   PHASE6_STACK_WORDS,
                                   PHASE6_LOW_PRIORITY);
    if (status != HR_OK)
    {
        board_uart_write_line("Low task creation failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_high_task_a,
                                   "high-a",
                                   high_priority_task_a,
                                   NULL,
                                   g_high_stack_a,
                                   PHASE6_STACK_WORDS,
                                   PHASE6_HIGH_PRIORITY);
    if (status != HR_OK)
    {
        board_uart_write_line("High task A creation failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_high_task_b,
                                   "high-b",
                                   high_priority_task_b,
                                   NULL,
                                   g_high_stack_b,
                                   PHASE6_STACK_WORDS,
                                   PHASE6_HIGH_PRIORITY);
    if (status != HR_OK)
    {
        board_uart_write_line("High task B creation failed.");
        board_panic();
    }

    /* Registration order intentionally starts with the lower-priority task. */
    if ((hr_task_start(&g_low_task) != HR_OK) ||
        (hr_task_start(&g_high_task_a) != HR_OK) ||
        (hr_task_start(&g_high_task_b) != HR_OK))
    {
        board_uart_write_line("Task registration failed.");
        board_panic();
    }

    board_uart_write_line("Starting scheduler through SVC...");
    status = hr_kernel_start();

    board_uart_write_string("ERROR: hr_kernel_start returned status=");
    board_uart_write_u32((uint32_t)status);
    board_uart_write_line("");
    board_panic();
    return (int)status;
}
