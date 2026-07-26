#include <stdint.h>

#include "board.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_time.h"
#include "hr_port.h"

#define PERIODIC_TASK_PRIORITY  2U
#define HEARTBEAT_TASK_PRIORITY 3U
#define TASK_STACK_WORDS        160U
#define PERIODIC_INTERVAL_TICKS       500U
#define HEARTBEAT_INTERVAL_TICKS    1000U

static hr_task_t g_periodic_task;
static hr_task_t g_heartbeat_task;
static hr_stack_t g_periodic_stack[TASK_STACK_WORDS];
static hr_stack_t g_heartbeat_stack[TASK_STACK_WORDS];

static void verify_task_context(const hr_task_t *expected)
{
    if ((hr_task_current() != expected) || !hr_port_thread_uses_psp())
    {
        board_uart_write_line("ERROR: invalid task context.");
        board_panic();
    }
}

static void periodic_task(void *argument)
{
    hr_tick_t release_tick = hr_time_now();
    uint32_t activation = 0U;

    (void)argument;

    for (;;)
    {
        verify_task_context(&g_periodic_task);
        activation++;
        board_led_toggle();
        board_uart_write_string("periodic activation=");
        board_uart_write_u32(activation);
        board_uart_write_string(" tick=");
        board_uart_write_u32(hr_time_now());
        board_uart_write_line(" -> delay_until +500");

        if (hr_task_delay_until(&release_tick, PERIODIC_INTERVAL_TICKS) != HR_OK)
        {
            board_uart_write_line("ERROR: periodic delay failed.");
            board_panic();
        }
    }
}

static void heartbeat_task(void *argument)
{
    uint32_t heartbeat = 0U;

    (void)argument;

    for (;;)
    {
        verify_task_context(&g_heartbeat_task);
        heartbeat++;
        board_uart_write_string("heartbeat activation=");
        board_uart_write_u32(heartbeat);
        board_uart_write_string(" tick=");
        board_uart_write_u32(hr_time_now());
        board_uart_write_line(" -> delay 1000");

        if (hr_task_delay(HEARTBEAT_INTERVAL_TICKS) != HR_OK)
        {
            board_uart_write_line("ERROR: heartbeat delay failed.");
            board_panic();
        }
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("hairtos SysTick and task delay");
    board_uart_write_line("SysTick drives the kernel tick and blocking delays.");
    board_uart_write_line("Tasks block; idle runs until a timeout expires.");

    status = hr_kernel_init();
    if (status != HR_OK)
    {
        board_uart_write_line("Kernel initialization failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_periodic_task,
                                   "periodic",
                                   periodic_task,
                                   NULL,
                                   g_periodic_stack,
                                   TASK_STACK_WORDS,
                                   PERIODIC_TASK_PRIORITY);
    if (status != HR_OK)
    {
        board_uart_write_line("Periodic task creation failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_heartbeat_task,
                                   "heartbeat",
                                   heartbeat_task,
                                   NULL,
                                   g_heartbeat_stack,
                                   TASK_STACK_WORDS,
                                   HEARTBEAT_TASK_PRIORITY);
    if (status != HR_OK)
    {
        board_uart_write_line("Heartbeat task creation failed.");
        board_panic();
    }

    if ((hr_task_start(&g_periodic_task) != HR_OK) ||
        (hr_task_start(&g_heartbeat_task) != HR_OK))
    {
        board_uart_write_line("Task registration failed.");
        board_panic();
    }

    board_uart_write_line("Starting first task through SVC...");
    status = hr_kernel_start();

    board_uart_write_string("ERROR: hr_kernel_start returned status=");
    board_uart_write_u32((uint32_t)status);
    board_uart_write_line("");
    board_panic();
    return (int)status;
}
