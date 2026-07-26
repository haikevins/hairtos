#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "board.h"
#include "hairtos/hairtos.h"

#ifndef HR_INJECT_USAGE_FAULT
#define HR_INJECT_USAGE_FAULT 0
#endif

#define HAIRTOS_QUEUE_CAPACITY     8U
#define HAIRTOS_PRODUCER_STACK     144U
#define HAIRTOS_CONSUMER_STACK     144U
#define HAIRTOS_PULSE_STACK        128U
#define HAIRTOS_MONITOR_STACK      224U
#define HAIRTOS_REPORT_PERIOD      1000U

static hr_queue_t g_queue;
static uint32_t g_queue_storage[HAIRTOS_QUEUE_CAPACITY];
static hr_semaphore_t g_pulse_semaphore;
static hr_mutex_t g_statistics_mutex;
static hr_timer_t g_pulse_timer;

static hr_task_t g_producer_task;
static hr_task_t g_consumer_task;
static hr_task_t g_pulse_task;
static hr_task_t g_monitor_task;
static hr_stack_t g_producer_stack[HAIRTOS_PRODUCER_STACK];
static hr_stack_t g_consumer_stack[HAIRTOS_CONSUMER_STACK];
static hr_stack_t g_pulse_stack[HAIRTOS_PULSE_STACK];
static hr_stack_t g_monitor_stack[HAIRTOS_MONITOR_STACK];

static uint32_t g_produced_count;
static uint32_t g_consumed_count;
static uint32_t g_send_timeout_count;
static uint32_t g_order_error_count;
static uint32_t g_pulse_count;

static void hairtos_require(hr_status_t status)
{
    if (status != HR_OK)
    {
        board_uart_write_string("hairtos setup failure status=");
        board_uart_write_u32((uint32_t)status);
        board_uart_write_line("");
        board_panic();
    }
}

static void hairtos_print_retained_panic(void)
{
    hr_panic_record_t record;

    hr_diagnostics_initialize();
    if (!hr_diagnostics_get_last_panic(&record))
    {
        board_uart_write_line("retained panic: none");
        return;
    }

    board_uart_write_string("retained panic: reason=");
    board_uart_write_string(hr_diagnostics_panic_reason_string(record.reason));
    board_uart_write_string(" sequence=");
    board_uart_write_u32(record.sequence);
    board_uart_write_string(" boot=");
    board_uart_write_u32(record.boot_count);
    board_uart_write_string(" task=");
    board_uart_write_string(record.task_name);
    board_uart_write_line("");

    if (record.fault.pc != 0U)
    {
        board_uart_write_string("fault pc=");
        board_uart_write_hex32(record.fault.pc);
        board_uart_write_string(" lr=");
        board_uart_write_hex32(record.fault.lr);
        board_uart_write_string(" cfsr=");
        board_uart_write_hex32(record.fault.cfsr);
        board_uart_write_line("");
    }

    hr_diagnostics_clear_last_panic();
}

static void hairtos_lock_statistics(void)
{
    hairtos_require(hr_mutex_lock(&g_statistics_mutex, HR_WAIT_FOREVER));
}

static void hairtos_unlock_statistics(void)
{
    hairtos_require(hr_mutex_unlock(&g_statistics_mutex));
}

static void hairtos_timer_callback(void *argument)
{
    (void)argument;

    if (hr_semaphore_give(&g_pulse_semaphore) == HR_ERROR_SEMAPHORE_FULL)
    {
        /* The pulse task deliberately tolerates coalesced timer notifications. */
    }
}

static void hairtos_producer(void *argument)
{
    uint32_t sequence = 0U;
    (void)argument;

    for (;;)
    {
        hr_status_t status;

        sequence++;
        status = hr_queue_send(&g_queue, &sequence, 10U);

        hairtos_lock_statistics();
        if (status == HR_OK)
        {
            g_produced_count++;
        }
        else if (status == HR_ERROR_TIMEOUT)
        {
            g_send_timeout_count++;
        }
        else
        {
            hairtos_unlock_statistics();
            board_panic();
        }
        hairtos_unlock_statistics();

        hairtos_require(hr_task_delay(2U));
    }
}

static void hairtos_consumer(void *argument)
{
    uint32_t last_sequence = 0U;
    (void)argument;

    for (;;)
    {
        uint32_t sequence;

        hairtos_require(hr_queue_receive(&g_queue,
                                         &sequence,
                                         HR_WAIT_FOREVER));

        hairtos_lock_statistics();
        if (sequence <= last_sequence)
        {
            g_order_error_count++;
        }
        last_sequence = sequence;
        g_consumed_count++;
        hairtos_unlock_statistics();

        hairtos_require(hr_task_delay(3U));
    }
}

static void hairtos_pulse_worker(void *argument)
{
    (void)argument;

    for (;;)
    {
        hairtos_require(hr_semaphore_take(&g_pulse_semaphore,
                                          HR_WAIT_FOREVER));
        hairtos_lock_statistics();
        g_pulse_count++;
        hairtos_unlock_statistics();
    }
}

static void hairtos_print_health(uint32_t report_index,
                                 const hr_health_report_t *health,
                                 const hr_runtime_statistics_t *runtime,
                                 uint32_t produced,
                                 uint32_t consumed,
                                 uint32_t timeouts,
                                 uint32_t pulses)
{
    board_uart_write_string("health report=");
    board_uart_write_u32(report_index);
    board_uart_write_string(" tick=");
    board_uart_write_u32(hr_time_now());
    board_uart_write_string(" tasks=");
    board_uart_write_u32((uint32_t)health->task_count);
    board_uart_write_string(" ready=");
    board_uart_write_u32((uint32_t)health->ready_task_count);
    board_uart_write_string(" timeout=");
    board_uart_write_u32((uint32_t)health->timeout_task_count);
    board_uart_write_string(" min_stack_free=");
    board_uart_write_u32((uint32_t)health->minimum_stack_free_words);
    board_uart_write_line("");

    board_uart_write_string("runtime switches=");
    board_uart_write_u32(runtime->task_switch_count);
    board_uart_write_string(" preemptions=");
    board_uart_write_u32(runtime->preemption_count);
    board_uart_write_string(" slices=");
    board_uart_write_u32(runtime->time_slice_count);
    board_uart_write_string(" blocks=");
    board_uart_write_u32(runtime->block_count);
    board_uart_write_line("");

    board_uart_write_string("stress produced=");
    board_uart_write_u32(produced);
    board_uart_write_string(" consumed=");
    board_uart_write_u32(consumed);
    board_uart_write_string(" send_timeouts=");
    board_uart_write_u32(timeouts);
    board_uart_write_string(" timer_pulses=");
    board_uart_write_u32(pulses);
    board_uart_write_line("");
}

static void hairtos_monitor(void *argument)
{
    hr_tick_t release_tick = hr_time_now();
    uint32_t report_index = 0U;
    uint32_t previous_consumed = 0U;
    (void)argument;

    for (;;)
    {
        hr_health_report_t health;
        hr_runtime_statistics_t runtime;
        uint32_t produced;
        uint32_t consumed;
        uint32_t timeouts;
        uint32_t order_errors;
        uint32_t pulses;

        hairtos_require(hr_task_delay_until(&release_tick,
                                            HAIRTOS_REPORT_PERIOD));
        report_index++;

        if (hr_diagnostics_run_health_check(&health) != HR_OK)
        {
            board_uart_write_line("hairtos health check: FAIL");
            board_panic();
        }
        hr_diagnostics_get_runtime_statistics(&runtime);

        hairtos_lock_statistics();
        produced = g_produced_count;
        consumed = g_consumed_count;
        timeouts = g_send_timeout_count;
        order_errors = g_order_error_count;
        pulses = g_pulse_count;
        hairtos_unlock_statistics();

        hairtos_print_health(report_index,
                             &health,
                             &runtime,
                             produced,
                             consumed,
                             timeouts,
                             pulses);

        if ((order_errors != 0U) ||
            ((report_index > 1U) && (consumed == previous_consumed)) ||
            !health.kernel_invariants_valid ||
            !health.all_stack_guards_valid)
        {
            board_uart_write_line("hairtos stabilization invariant: FAIL");
            board_panic();
        }
        previous_consumed = consumed;

#if (HR_INJECT_USAGE_FAULT == 1)
        if (report_index == 5U)
        {
            board_uart_write_line("Injecting UsageFault; reset to inspect record");
            __asm volatile ("udf #0");
        }
#endif

        if (report_index == 10U)
        {
            board_uart_write_line("hairtos diagnostics/stress: PASS (10 s checkpoint)");
        }

        board_led_toggle();
    }
}

void hr_hook_panic(const hr_panic_record_t *record)
{
    (void)record;
    board_led_on();
}

void hr_hook_stack_overflow(const hr_task_t *task, const char *task_name)
{
    (void)task;
    (void)task_name;
    board_led_on();
}

int main(void)
{
    board_init();
    board_uart_write_line("hairtos diagnostics and stabilization");
    board_uart_write_line("Retained faults, runtime counters, health checks, and stress workload.");
    hairtos_print_retained_panic();

    hairtos_require(hr_kernel_init());
    hairtos_require(hr_queue_create_static(&g_queue,
                                           g_queue_storage,
                                           sizeof(g_queue_storage[0]),
                                           HAIRTOS_QUEUE_CAPACITY));
    hairtos_require(hr_semaphore_create_counting(&g_pulse_semaphore,
                                                 0U,
                                                 16U));
    hairtos_require(hr_mutex_create(&g_statistics_mutex));
    hairtos_require(hr_timer_create_static(&g_pulse_timer,
                                           "diagnostic-pulse",
                                           10U,
                                           true,
                                           hairtos_timer_callback,
                                           NULL));

    hairtos_require(hr_task_create_static(&g_monitor_task,
                                          "health-monitor",
                                          hairtos_monitor,
                                          NULL,
                                          g_monitor_stack,
                                          HAIRTOS_MONITOR_STACK,
                                          1U));
    hairtos_require(hr_task_create_static(&g_consumer_task,
                                          "queue-consumer",
                                          hairtos_consumer,
                                          NULL,
                                          g_consumer_stack,
                                          HAIRTOS_CONSUMER_STACK,
                                          2U));
    hairtos_require(hr_task_create_static(&g_pulse_task,
                                          "timer-pulse",
                                          hairtos_pulse_worker,
                                          NULL,
                                          g_pulse_stack,
                                          HAIRTOS_PULSE_STACK,
                                          2U));
    hairtos_require(hr_task_create_static(&g_producer_task,
                                          "queue-producer",
                                          hairtos_producer,
                                          NULL,
                                          g_producer_stack,
                                          HAIRTOS_PRODUCER_STACK,
                                          3U));

    hairtos_require(hr_task_start(&g_monitor_task));
    hairtos_require(hr_task_start(&g_consumer_task));
    hairtos_require(hr_task_start(&g_pulse_task));
    hairtos_require(hr_task_start(&g_producer_task));
    hairtos_require(hr_timer_start(&g_pulse_timer));

    board_uart_write_line("Starting long-duration stress workload through SVC...");
    hairtos_require(hr_kernel_start());
    board_panic();
    return 0;
}
