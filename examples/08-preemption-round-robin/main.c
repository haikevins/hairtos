#include <stdint.h>

#include "board.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_time.h"
#include "hr_port.h"

#define PHASE8_MONITOR_PRIORITY      1U
#define PHASE8_WORKER_PRIORITY       3U
#define PHASE8_STACK_WORDS           192U
#define PHASE8_MONITOR_PERIOD_TICKS  250U

static hr_task_t g_monitor_task;
static hr_task_t g_worker_a_task;
static hr_task_t g_worker_b_task;
static hr_stack_t g_monitor_stack[PHASE8_STACK_WORDS];
static hr_stack_t g_worker_a_stack[PHASE8_STACK_WORDS];
static hr_stack_t g_worker_b_stack[PHASE8_STACK_WORDS];

static volatile uint32_t g_worker_a_counter;
static volatile uint32_t g_worker_b_counter;

static void phase8_verify_context(const hr_task_t *expected)
{
    if ((hr_task_current() != expected) || !hr_port_thread_uses_psp())
    {
        board_uart_write_line("ERROR: invalid task context.");
        board_panic();
    }
}

static void worker_a_task(void *argument)
{
    (void)argument;

    for (;;)
    {
        g_worker_a_counter++;
    }
}

static void worker_b_task(void *argument)
{
    (void)argument;

    for (;;)
    {
        g_worker_b_counter++;
    }
}

static void monitor_task(void *argument)
{
    hr_tick_t release_tick = hr_time_now();
    uint32_t previous_a = 0U;
    uint32_t previous_b = 0U;
    uint32_t activation = 0U;

    (void)argument;

    for (;;)
    {
        uint32_t snapshot_a;
        uint32_t snapshot_b;

        phase8_verify_context(&g_monitor_task);
        activation++;
        snapshot_a = g_worker_a_counter;
        snapshot_b = g_worker_b_counter;

        if ((activation > 1U) &&
            ((snapshot_a <= previous_a) || (snapshot_b <= previous_b)))
        {
            board_uart_write_line("ERROR: equal-priority worker starvation detected.");
            board_panic();
        }

        board_led_toggle();
        board_uart_write_string("monitor preempted workers at tick=");
        board_uart_write_u32(hr_time_now());
        board_uart_write_string(" worker-a=");
        board_uart_write_u32(snapshot_a);
        board_uart_write_string(" worker-b=");
        board_uart_write_u32(snapshot_b);
        board_uart_write_line("");

        previous_a = snapshot_a;
        previous_b = snapshot_b;

        if (hr_task_delay_until(&release_tick,
                                PHASE8_MONITOR_PERIOD_TICKS) != HR_OK)
        {
            board_uart_write_line("ERROR: monitor delay failed.");
            board_panic();
        }
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("hairtos Phase 8");
    board_uart_write_line("Two CPU-bound equal-priority workers never call yield().");
    board_uart_write_line("SysTick round-robin shares CPU; monitor wake-up preempts them.");

    status = hr_kernel_init();
    if (status != HR_OK)
    {
        board_uart_write_line("Kernel initialization failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_monitor_task,
                                   "monitor",
                                   monitor_task,
                                   NULL,
                                   g_monitor_stack,
                                   PHASE8_STACK_WORDS,
                                   PHASE8_MONITOR_PRIORITY);
    if (status != HR_OK)
    {
        board_uart_write_line("Monitor task creation failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_worker_a_task,
                                   "worker-a",
                                   worker_a_task,
                                   NULL,
                                   g_worker_a_stack,
                                   PHASE8_STACK_WORDS,
                                   PHASE8_WORKER_PRIORITY);
    if (status != HR_OK)
    {
        board_uart_write_line("Worker A creation failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_worker_b_task,
                                   "worker-b",
                                   worker_b_task,
                                   NULL,
                                   g_worker_b_stack,
                                   PHASE8_STACK_WORDS,
                                   PHASE8_WORKER_PRIORITY);
    if (status != HR_OK)
    {
        board_uart_write_line("Worker B creation failed.");
        board_panic();
    }

    if ((hr_task_start(&g_worker_a_task) != HR_OK) ||
        (hr_task_start(&g_worker_b_task) != HR_OK) ||
        (hr_task_start(&g_monitor_task) != HR_OK))
    {
        board_uart_write_line("Task registration failed.");
        board_panic();
    }

    board_uart_write_line("Starting preemptive scheduler through SVC...");
    status = hr_kernel_start();

    board_uart_write_string("ERROR: hr_kernel_start returned status=");
    board_uart_write_u32((uint32_t)status);
    board_uart_write_line("");
    board_panic();
    return (int)status;
}
