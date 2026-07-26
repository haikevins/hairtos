#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_time.h"
#include "hairtos/hr_timer.h"
#include "hr_port.h"

#define CONTROL_TASK_PRIORITY  3U
#define CONTROL_TASK_STACK_WORDS       224U

static hr_timer_t g_periodic_timer;
static hr_timer_t g_one_shot_timer;
static hr_task_t g_control_task;
static hr_stack_t g_control_stack[CONTROL_TASK_STACK_WORDS];
static volatile uint32_t g_periodic_callbacks;
static volatile uint32_t g_one_shot_callbacks;

static void periodic_callback(void *argument)
{
    (void)argument;
    g_periodic_callbacks++;
    board_led_toggle();
    board_uart_write_string("timer-service: periodic callback=");
    board_uart_write_u32(g_periodic_callbacks);
    board_uart_write_string(" tick=");
    board_uart_write_u32(hr_time_now());
    board_uart_write_line("");

    if (g_periodic_callbacks == 4U)
    {
        board_uart_write_line("timer-service: change periodic period 250 -> 500 ticks");
        if (hr_timer_change_period(&g_periodic_timer, 500U) != HR_OK)
        {
            board_panic();
        }
    }
}

static void one_shot_callback(void *argument)
{
    const uint32_t magic = *(const uint32_t *)argument;
    g_one_shot_callbacks++;
    board_uart_write_string("timer-service: one-shot callback tick=");
    board_uart_write_u32(hr_time_now());
    board_uart_write_string(" argument=");
    board_uart_write_u32(magic);
    board_uart_write_line("");
}

static void control_task(void *argument)
{
    (void)argument;

    if ((hr_task_current() != &g_control_task) || !hr_port_thread_uses_psp())
    {
        board_panic();
    }

    board_uart_write_line("control: start periodic 250-tick and one-shot 1000-tick timers");
    if ((hr_timer_start(&g_periodic_timer) != HR_OK) ||
        (hr_timer_start(&g_one_shot_timer) != HR_OK))
    {
        board_panic();
    }

    if (hr_task_delay(400U) != HR_OK)
    {
        board_panic();
    }

    board_uart_write_line("control: reset one-shot; deadline moves 1000 -> 1400 ticks");
    if (hr_timer_reset(&g_one_shot_timer) != HR_OK)
    {
        board_panic();
    }

    if (hr_task_delay(2200U) != HR_OK)
    {
        board_panic();
    }

    if ((g_one_shot_callbacks != 1U) || (g_periodic_callbacks < 6U))
    {
        board_uart_write_line("ERROR: software timer callback count mismatch.");
        board_panic();
    }

    if (hr_timer_stop(&g_periodic_timer) != HR_OK)
    {
        board_panic();
    }

    board_uart_write_line("Software timer service: PASS");
    for (;;)
    {
        (void)hr_task_delay(1000U);
    }
}

int main(void)
{
    static uint32_t one_shot_magic = UINT32_C(120012);
    hr_status_t status;

    board_init();
    board_uart_write_line("hairtos software timer");
    board_uart_write_line("SysTick only schedules expirations; callbacks run in timer-service task context.");

    status = hr_kernel_init();
    if (status != HR_OK)
    {
        board_panic();
    }

    if ((hr_timer_create_static(&g_periodic_timer,
                                "periodic",
                                250U,
                                true,
                                periodic_callback,
                                NULL) != HR_OK) ||
        (hr_timer_create_static(&g_one_shot_timer,
                                "one-shot",
                                1000U,
                                false,
                                one_shot_callback,
                                &one_shot_magic) != HR_OK) ||
        (hr_task_create_static(&g_control_task,
                               "timer-control",
                               control_task,
                               NULL,
                               g_control_stack,
                               CONTROL_TASK_STACK_WORDS,
                               CONTROL_TASK_PRIORITY) != HR_OK) ||
        (hr_task_start(&g_control_task) != HR_OK))
    {
        board_uart_write_line("Software timer setup failed.");
        board_panic();
    }

    board_uart_write_line("Starting timer service and control task through SVC...");
    status = hr_kernel_start();
    board_uart_write_u32((uint32_t)status);
    board_panic();
    return (int)status;
}
