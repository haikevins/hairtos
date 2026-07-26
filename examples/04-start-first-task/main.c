#include <stdint.h>

#include "board.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hr_port.h"

#define FIRST_TASK_ARGUMENT_MAGIC 0x50483421UL

static hr_task_t g_first_task;
static hr_stack_t g_first_task_stack[128];
static uint32_t g_first_task_argument = FIRST_TASK_ARGUMENT_MAGIC;

static void first_task(void *argument)
{
    const uint32_t received_magic = *(const uint32_t *)argument;
    uint32_t heartbeat = 0U;

    board_uart_write_line("First task entered through SVC.");
    board_uart_write_string("Current task: ");
    board_uart_write_line(hr_task_get_name(hr_task_current()));

    if (!hr_port_thread_uses_psp())
    {
        board_uart_write_line("ERROR: Thread mode is not using PSP.");
        board_panic();
    }

    if (received_magic != FIRST_TASK_ARGUMENT_MAGIC)
    {
        board_uart_write_line("ERROR: task argument was not restored in R0.");
        board_panic();
    }

    board_uart_write_line("PSP active: yes");
    board_uart_write_line("Task argument: valid");
    board_uart_write_line("First-task startup: PASS");

    for (;;)
    {
        heartbeat++;
        board_led_toggle();
        board_uart_write_string("first-task heartbeat=");
        board_uart_write_u32(heartbeat);
        board_uart_write_line("");
        board_delay_ms(500U);
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("hairtos first-task startup");
    board_uart_write_line("Preparing idle task and first application task...");

    status = hr_kernel_init();
    if (status != HR_OK)
    {
        board_uart_write_line("Kernel initialization failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_first_task,
                                   "first-task",
                                   first_task,
                                   &g_first_task_argument,
                                   g_first_task_stack,
                                   128U,
                                   2U);
    if (status != HR_OK)
    {
        board_uart_write_line("First task creation failed.");
        board_panic();
    }

    status = hr_task_start(&g_first_task);
    if (status != HR_OK)
    {
        board_uart_write_line("First task registration failed.");
        board_panic();
    }

    board_uart_write_line("Invoking SVC to leave main/MSP and enter task/PSP...");
    status = hr_kernel_start();

    board_uart_write_string("ERROR: hr_kernel_start returned status=");
    board_uart_write_u32((uint32_t)status);
    board_uart_write_line("");
    board_panic();
    return (int)status;
}
