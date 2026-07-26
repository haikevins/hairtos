#include <stdint.h>

#include "board.h"
#include "hairtos/hr_task.h"

static hr_task_t g_demo_task;
static hr_stack_t g_demo_stack[96];

static void demo_task(void *argument)
{
    volatile uint32_t *counter = (volatile uint32_t *)argument;

    for (;;)
    {
        (*counter)++;
    }
}

int main(void)
{
    static volatile uint32_t counter = 0U;
    hr_status_t status;

    board_init();
    board_uart_write_line("hairtos static task stack");
    board_uart_write_line("Creating a static TCB and Cortex-M3 initial stack frame...");

    status = hr_task_create_static(&g_demo_task,
                                   "demo",
                                   demo_task,
                                   (void *)&counter,
                                   g_demo_stack,
                                   96U,
                                   2U);

    if (status == HR_OK)
    {
        board_uart_write_line("Task object: CREATED");
        board_uart_write_line("Initial stack frame: READY");
        board_uart_write_line("Task execution starts in the SVC startup example.");
    }
    else
    {
        board_uart_write_line("Task creation failed.");
    }

    for (;;)
    {
        board_led_toggle();
        board_delay_ms(500U);
    }
}
