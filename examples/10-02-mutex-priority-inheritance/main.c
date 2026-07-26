#include <stdint.h>

#include "board.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_mutex.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_time.h"
#include "hr_port.h"

#define HIGH_TASK_PRIORITY       1U
#define MEDIUM_TASK_PRIORITY     3U
#define LOW_TASK_PRIORITY        5U
#define TASK_STACK_WORDS         224U
#define HIGH_TASK_RELEASE_TICK         10U
#define MEDIUM_TASK_RELEASE_TICK       20U
#define LOW_TASK_WORK_UNTIL_TICK       120U

static hr_mutex_t g_resource_mutex;
static hr_task_t g_high_task;
static hr_task_t g_medium_task;
static hr_task_t g_low_task;
static hr_stack_t g_high_stack[TASK_STACK_WORDS];
static hr_stack_t g_medium_stack[TASK_STACK_WORDS];
static hr_stack_t g_low_stack[TASK_STACK_WORDS];
static volatile uint32_t g_medium_work;
static volatile bool g_demo_passed;

static void high_task(void *argument)
{
    (void)argument;

    if (hr_task_delay(HIGH_TASK_RELEASE_TICK) != HR_OK)
    {
        board_panic();
    }

    board_uart_write_line("high: attempts mutex and blocks");
    if (hr_mutex_lock(&g_resource_mutex, HR_WAIT_FOREVER) != HR_OK)
    {
        board_uart_write_line("ERROR: high mutex lock failed.");
        board_panic();
    }

    if ((hr_mutex_get_owner(&g_resource_mutex) != &g_high_task) ||
        (hr_task_get_effective_priority(&g_low_task) !=
         LOW_TASK_PRIORITY))
    {
        board_uart_write_line("ERROR: mutex ownership/restoration failed.");
        board_panic();
    }

    g_demo_passed = true;
    board_led_toggle();
    board_uart_write_string("high: acquired at tick=");
    board_uart_write_u32(hr_time_now());
    board_uart_write_string(" medium_work=");
    board_uart_write_u32(g_medium_work);
    board_uart_write_line(" PASS");

    if (hr_mutex_unlock(&g_resource_mutex) != HR_OK)
    {
        board_panic();
    }

    for (;;)
    {
        (void)hr_task_delay(1000U);
    }
}

static void medium_task(void *argument)
{
    (void)argument;

    if (hr_task_delay(MEDIUM_TASK_RELEASE_TICK) != HR_OK)
    {
        board_panic();
    }

    while (!g_demo_passed)
    {
        g_medium_work++;
    }

    for (;;)
    {
        (void)hr_task_delay(1000U);
    }
}

static void low_task(void *argument)
{
    (void)argument;

    if (hr_mutex_lock(&g_resource_mutex, HR_NO_WAIT) != HR_OK)
    {
        board_uart_write_line("ERROR: low could not acquire mutex first.");
        board_panic();
    }

    board_uart_write_line("low: owns mutex at base priority 5");

    while (hr_time_now() < LOW_TASK_WORK_UNTIL_TICK)
    {
        if ((hr_time_now() >= HIGH_TASK_RELEASE_TICK) &&
            (hr_mutex_get_waiting_tasks(&g_resource_mutex) > 0U) &&
            (hr_task_get_effective_priority(&g_low_task) !=
             HIGH_TASK_PRIORITY))
        {
            board_uart_write_line("ERROR: low did not inherit priority 1.");
            board_panic();
        }
    }

    board_uart_write_string("low: inherited priority=");
    board_uart_write_u32(hr_task_get_effective_priority(&g_low_task));
    board_uart_write_line("; unlocking");

    if (hr_mutex_unlock(&g_resource_mutex) != HR_OK)
    {
        board_uart_write_line("ERROR: low mutex unlock failed.");
        board_panic();
    }

    for (;;)
    {
        (void)hr_task_delay(1000U);
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("hairtos mutex priority inheritance");
    board_uart_write_line("Priority inversion corrected by mutex inheritance.");
    board_uart_write_line("High=1, Medium=3, Low=5.");

    if (hr_mutex_create(&g_resource_mutex) != HR_OK)
    {
        board_uart_write_line("Mutex creation failed.");
        board_panic();
    }

    status = hr_kernel_init();
    if (status != HR_OK)
    {
        board_panic();
    }

    if ((hr_task_create_static(&g_high_task,
                               "high",
                               high_task,
                               NULL,
                               g_high_stack,
                               TASK_STACK_WORDS,
                               HIGH_TASK_PRIORITY) != HR_OK) ||
        (hr_task_create_static(&g_medium_task,
                               "medium",
                               medium_task,
                               NULL,
                               g_medium_stack,
                               TASK_STACK_WORDS,
                               MEDIUM_TASK_PRIORITY) != HR_OK) ||
        (hr_task_create_static(&g_low_task,
                               "low",
                               low_task,
                               NULL,
                               g_low_stack,
                               TASK_STACK_WORDS,
                               LOW_TASK_PRIORITY) != HR_OK) ||
        (hr_task_start(&g_low_task) != HR_OK) ||
        (hr_task_start(&g_medium_task) != HR_OK) ||
        (hr_task_start(&g_high_task) != HR_OK))
    {
        board_uart_write_line("Mutex task setup failed.");
        board_panic();
    }

    board_uart_write_line("Starting mutex scheduler through SVC...");
    status = hr_kernel_start();
    board_uart_write_u32((uint32_t)status);
    board_panic();
    return (int)status;
}
