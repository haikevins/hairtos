#include <stdint.h>

#include "board.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_time.h"
#include "hr_port.h"

#define PHASE11_WORKER_PRIORITY       1U
#define PHASE11_SUPERVISOR_PRIORITY   2U
#define PHASE11_BACKGROUND_PRIORITY   4U
#define PHASE11_STACK_WORDS           224U

static hr_task_t g_worker_task;
static hr_task_t g_supervisor_task;
static hr_task_t g_background_task;
static hr_stack_t g_worker_stack[PHASE11_STACK_WORDS];
static hr_stack_t g_supervisor_stack[PHASE11_STACK_WORDS];
static hr_stack_t g_background_stack[PHASE11_STACK_WORDS];
static volatile uint32_t g_background_counter;
static volatile bool g_worker_reached_self_suspend;

static void verify_task_context(const hr_task_t *expected)
{
    if ((hr_task_current() != expected) || !hr_port_thread_uses_psp())
    {
        board_uart_write_line("ERROR: invalid Phase 11 task context.");
        board_panic();
    }
}

static void worker_task(void *argument)
{
    (void)argument;
    verify_task_context(&g_worker_task);
    board_uart_write_line("worker: delay 100 ticks");

    if (hr_task_delay(100U) != HR_OK)
    {
        board_panic();
    }

    verify_task_context(&g_worker_task);
    board_uart_write_string("worker: timeout completed while suspended; resumed at tick=");
    board_uart_write_u32(hr_time_now());
    board_uart_write_line("");

    g_worker_reached_self_suspend = true;
    board_uart_write_line("worker: self-suspend");
    if (hr_task_suspend(&g_worker_task) != HR_OK)
    {
        board_panic();
    }

    verify_task_context(&g_worker_task);
    board_uart_write_string("worker: self-resume PASS at tick=");
    board_uart_write_u32(hr_time_now());
    board_uart_write_line("");
    board_led_toggle();

    for (;;)
    {
        (void)hr_task_delay(1000U);
    }
}

static void supervisor_task(void *argument)
{
    (void)argument;
    verify_task_context(&g_supervisor_task);

    if (hr_task_delay(50U) != HR_OK)
    {
        board_panic();
    }

    board_uart_write_string("supervisor: suspend blocked worker at tick=");
    board_uart_write_u32(hr_time_now());
    board_uart_write_line("");
    if ((hr_task_get_state(&g_worker_task) != HR_TASK_STATE_BLOCKED) ||
        (hr_task_suspend(&g_worker_task) != HR_OK))
    {
        board_panic();
    }

    if (hr_task_delay(200U) != HR_OK)
    {
        board_panic();
    }

    if (hr_task_get_state(&g_worker_task) != HR_TASK_STATE_SUSPENDED)
    {
        board_uart_write_line("ERROR: timeout made suspended worker READY.");
        board_panic();
    }

    board_uart_write_string("supervisor: resume high-priority worker at tick=");
    board_uart_write_u32(hr_time_now());
    board_uart_write_line("");
    if (hr_task_resume(&g_worker_task) != HR_OK)
    {
        board_panic();
    }

    /* The resumed priority-1 worker preempts here and self-suspends. */
    if (!g_worker_reached_self_suspend)
    {
        board_uart_write_line("ERROR: resumed worker did not preempt supervisor.");
        board_panic();
    }

    if (hr_task_delay(100U) != HR_OK)
    {
        board_panic();
    }

    board_uart_write_line("supervisor: resume self-suspended worker");
    if (hr_task_resume(&g_worker_task) != HR_OK)
    {
        board_panic();
    }

    for (;;)
    {
        (void)hr_task_delay(1000U);
    }
}

static void background_task(void *argument)
{
    (void)argument;
    for (;;)
    {
        g_background_counter++;
        if ((g_background_counter & UINT32_C(0x3FFFF)) == 0U)
        {
            board_led_toggle();
        }
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("HairRTOS Phase 11");
    board_uart_write_line("Suspend/resume for READY, RUNNING, and BLOCKED tasks.");
    board_uart_write_line("A suspended timeout completes but cannot make the task READY.");

    status = hr_kernel_init();
    if (status != HR_OK)
    {
        board_panic();
    }

    if ((hr_task_create_static(&g_worker_task,
                               "worker",
                               worker_task,
                               NULL,
                               g_worker_stack,
                               PHASE11_STACK_WORDS,
                               PHASE11_WORKER_PRIORITY) != HR_OK) ||
        (hr_task_create_static(&g_supervisor_task,
                               "supervisor",
                               supervisor_task,
                               NULL,
                               g_supervisor_stack,
                               PHASE11_STACK_WORDS,
                               PHASE11_SUPERVISOR_PRIORITY) != HR_OK) ||
        (hr_task_create_static(&g_background_task,
                               "background",
                               background_task,
                               NULL,
                               g_background_stack,
                               PHASE11_STACK_WORDS,
                               PHASE11_BACKGROUND_PRIORITY) != HR_OK) ||
        (hr_task_start(&g_background_task) != HR_OK) ||
        (hr_task_start(&g_supervisor_task) != HR_OK) ||
        (hr_task_start(&g_worker_task) != HR_OK))
    {
        board_uart_write_line("Phase 11 task setup failed.");
        board_panic();
    }

    board_uart_write_line("Starting Phase 11 scheduler through SVC...");
    status = hr_kernel_start();
    board_uart_write_u32((uint32_t)status);
    board_panic();
    return (int)status;
}
