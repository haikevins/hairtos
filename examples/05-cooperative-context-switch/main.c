#include <stdint.h>

#include "board.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hr_port.h"

#define PHASE5_TASK_PRIORITY 2U
#define PHASE5_STACK_WORDS   160U
#define PHASE5_DELAY_MS      250U

static hr_task_t g_task_a;
static hr_task_t g_task_b;
static hr_stack_t g_task_a_stack[PHASE5_STACK_WORDS];
static hr_stack_t g_task_b_stack[PHASE5_STACK_WORDS];

static void phase5_verify_current(const hr_task_t *expected, const char *label)
{
    if (hr_task_current() != expected)
    {
        board_uart_write_string("ERROR: wrong current task in ");
        board_uart_write_line(label);
        board_panic();
    }

    if (!hr_port_thread_uses_psp())
    {
        board_uart_write_line("ERROR: cooperative task is not using PSP.");
        board_panic();
    }
}

static void task_a(void *argument)
{
    uint32_t local_counter = 0U;
    uint32_t stack_cookie[2] = {0xA5A50001UL, 0xA5A50002UL};

    (void)argument;

    for (;;)
    {
        phase5_verify_current(&g_task_a, "task A");
        if ((stack_cookie[0] != 0xA5A50001UL) ||
            (stack_cookie[1] != 0xA5A50002UL))
        {
            board_uart_write_line("ERROR: task A stack-local state was corrupted.");
            board_panic();
        }

        local_counter++;
        board_led_toggle();
        board_uart_write_string("task=A local_counter=");
        board_uart_write_u32(local_counter);
        board_uart_write_line(" -> yield");

        /* Temporary Phase 1 busy wait; blocking delay arrives in Phase 7. */
        board_delay_ms(PHASE5_DELAY_MS);
        hr_task_yield();
    }
}

static void task_b(void *argument)
{
    uint32_t local_counter = 1000U;
    uint32_t stack_cookie[2] = {0xB5B50001UL, 0xB5B50002UL};

    (void)argument;

    for (;;)
    {
        phase5_verify_current(&g_task_b, "task B");
        if ((stack_cookie[0] != 0xB5B50001UL) ||
            (stack_cookie[1] != 0xB5B50002UL))
        {
            board_uart_write_line("ERROR: task B stack-local state was corrupted.");
            board_panic();
        }

        local_counter += 10U;
        board_uart_write_string("task=B local_counter=");
        board_uart_write_u32(local_counter);
        board_uart_write_line(" -> yield");

        board_delay_ms(PHASE5_DELAY_MS);
        hr_task_yield();
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("hairtos Phase 5");
    board_uart_write_line("Two equal-priority tasks switch cooperatively through PendSV.");

    status = hr_kernel_init();
    if (status != HR_OK)
    {
        board_uart_write_line("Kernel initialization failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_task_a,
                                   "task-a",
                                   task_a,
                                   NULL,
                                   g_task_a_stack,
                                   PHASE5_STACK_WORDS,
                                   PHASE5_TASK_PRIORITY);
    if (status != HR_OK)
    {
        board_uart_write_line("Task A creation failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_task_b,
                                   "task-b",
                                   task_b,
                                   NULL,
                                   g_task_b_stack,
                                   PHASE5_STACK_WORDS,
                                   PHASE5_TASK_PRIORITY);
    if (status != HR_OK)
    {
        board_uart_write_line("Task B creation failed.");
        board_panic();
    }

    if ((hr_task_start(&g_task_a) != HR_OK) ||
        (hr_task_start(&g_task_b) != HR_OK))
    {
        board_uart_write_line("Task registration failed.");
        board_panic();
    }

    board_uart_write_line("Starting task A through SVC; each yield pends PendSV.");
    status = hr_kernel_start();

    board_uart_write_string("ERROR: hr_kernel_start returned status=");
    board_uart_write_u32((uint32_t)status);
    board_uart_write_line("");
    board_panic();
    return (int)status;
}
