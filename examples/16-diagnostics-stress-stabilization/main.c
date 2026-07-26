#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "board.h"
#include "hairtos/hairtos.h"

#ifndef HR_DIAGNOSTICS_INJECT_USAGE_FAULT
#define HR_DIAGNOSTICS_INJECT_USAGE_FAULT 0
#endif

#define MESSAGE_QUEUE_CAPACITY       8U
#define PRODUCER_STACK_WORDS        144U
#define CONSUMER_STACK_WORDS        144U
#define PULSE_TASK_STACK_WORDS      128U
#define HEALTH_MONITOR_STACK_WORDS  224U
#define HEALTH_REPORT_PERIOD_TICKS 1000U

static hr_queue_t g_message_queue;
static uint32_t g_message_storage[MESSAGE_QUEUE_CAPACITY];
static hr_semaphore_t g_pulse_notification;
static hr_mutex_t g_statistics_lock;
static hr_timer_t g_diagnostic_timer;

static hr_task_t g_producer_task;
static hr_task_t g_consumer_task;
static hr_task_t g_pulse_task;
static hr_task_t g_health_monitor_task;

static hr_stack_t g_producer_stack[PRODUCER_STACK_WORDS];
static hr_stack_t g_consumer_stack[CONSUMER_STACK_WORDS];
static hr_stack_t g_pulse_stack[PULSE_TASK_STACK_WORDS];
static hr_stack_t g_health_monitor_stack[HEALTH_MONITOR_STACK_WORDS];

static uint32_t g_produced_count;
static uint32_t g_consumed_count;
static uint32_t g_send_timeout_count;
static uint32_t g_order_error_count;
static uint32_t g_pulse_count;

static void require_ok(hr_status_t status)
{
    if (status != HR_OK)
    {
        board_uart_write_string("Diagnostics setup failure status=");
        board_uart_write_u32((uint32_t)status);
        board_uart_write_line("");
        board_panic();
    }
}

static void print_retained_panic(void)
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

static void lock_statistics(void)
{
    require_ok(hr_mutex_lock(&g_statistics_lock, HR_WAIT_FOREVER));
}

static void unlock_statistics(void)
{
    require_ok(hr_mutex_unlock(&g_statistics_lock));
}

static void pulse_timer_callback(void *argument)
{
    (void)argument;

    if (hr_semaphore_give(&g_pulse_notification) ==
        HR_ERROR_SEMAPHORE_FULL)
    {
        /* Coalesced timer notifications are acceptable in this workload. */
    }
}

static void producer_task_entry(void *argument)
{
    uint32_t sequence = 0U;
    (void)argument;

    for (;;)
    {
        hr_status_t status;

        sequence++;
        status = hr_queue_send(&g_message_queue, &sequence, 10U);

        lock_statistics();

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
            unlock_statistics();
            board_panic();
        }

        unlock_statistics();
        require_ok(hr_task_delay(2U));
    }
}

static void consumer_task_entry(void *argument)
{
    uint32_t last_sequence = 0U;
    (void)argument;

    for (;;)
    {
        uint32_t sequence;

        require_ok(hr_queue_receive(&g_message_queue,
                                    &sequence,
                                    HR_WAIT_FOREVER));

        lock_statistics();

        if (sequence <= last_sequence)
        {
            g_order_error_count++;
        }

        last_sequence = sequence;
        g_consumed_count++;

        unlock_statistics();
        require_ok(hr_task_delay(3U));
    }
}

static void pulse_task_entry(void *argument)
{
    (void)argument;

    for (;;)
    {
        require_ok(hr_semaphore_take(&g_pulse_notification,
                                     HR_WAIT_FOREVER));

        lock_statistics();
        g_pulse_count++;
        unlock_statistics();
    }
}

static void print_health_report(uint32_t report_index,
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

static void health_monitor_task_entry(void *argument)
{
    hr_tick_t next_report_tick = hr_time_now();
    uint32_t report_index = 0U;
    uint32_t previous_consumed_count = 0U;
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

        require_ok(hr_task_delay_until(&next_report_tick,
                                       HEALTH_REPORT_PERIOD_TICKS));
        report_index++;

        if (hr_diagnostics_run_health_check(&health) != HR_OK)
        {
            board_uart_write_line("Diagnostics health check: FAIL");
            board_panic();
        }

        hr_diagnostics_get_runtime_statistics(&runtime);

        lock_statistics();
        produced = g_produced_count;
        consumed = g_consumed_count;
        timeouts = g_send_timeout_count;
        order_errors = g_order_error_count;
        pulses = g_pulse_count;
        unlock_statistics();

        print_health_report(report_index,
                            &health,
                            &runtime,
                            produced,
                            consumed,
                            timeouts,
                            pulses);

        if ((order_errors != 0U) ||
            ((report_index > 1U) &&
             (consumed == previous_consumed_count)) ||
            !health.kernel_invariants_valid ||
            !health.all_stack_guards_valid)
        {
            board_uart_write_line(
                "Diagnostics stabilization invariant: FAIL");
            board_panic();
        }

        previous_consumed_count = consumed;

#if (HR_DIAGNOSTICS_INJECT_USAGE_FAULT == 1)
        if (report_index == 5U)
        {
            board_uart_write_line(
                "Injecting UsageFault; reset to inspect record");
            __asm volatile ("udf #0");
        }
#endif

        if (report_index == 10U)
        {
            board_uart_write_line(
                "Diagnostics/stress: PASS (10 s checkpoint)");
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
    board_uart_write_line(
        "Retained faults, runtime counters, health checks, and stress workload.");
    print_retained_panic();

    require_ok(hr_kernel_init());
    require_ok(hr_queue_create_static(&g_message_queue,
                                      g_message_storage,
                                      sizeof(g_message_storage[0]),
                                      MESSAGE_QUEUE_CAPACITY));
    require_ok(hr_semaphore_create_counting(&g_pulse_notification,
                                            0U,
                                            16U));
    require_ok(hr_mutex_create(&g_statistics_lock));
    require_ok(hr_timer_create_static(&g_diagnostic_timer,
                                      "diagnostic-pulse",
                                      10U,
                                      true,
                                      pulse_timer_callback,
                                      NULL));

    require_ok(hr_task_create_static(&g_health_monitor_task,
                                     "health-monitor",
                                     health_monitor_task_entry,
                                     NULL,
                                     g_health_monitor_stack,
                                     HEALTH_MONITOR_STACK_WORDS,
                                     1U));
    require_ok(hr_task_create_static(&g_consumer_task,
                                     "queue-consumer",
                                     consumer_task_entry,
                                     NULL,
                                     g_consumer_stack,
                                     CONSUMER_STACK_WORDS,
                                     2U));
    require_ok(hr_task_create_static(&g_pulse_task,
                                     "timer-pulse",
                                     pulse_task_entry,
                                     NULL,
                                     g_pulse_stack,
                                     PULSE_TASK_STACK_WORDS,
                                     2U));
    require_ok(hr_task_create_static(&g_producer_task,
                                     "queue-producer",
                                     producer_task_entry,
                                     NULL,
                                     g_producer_stack,
                                     PRODUCER_STACK_WORDS,
                                     3U));

    require_ok(hr_task_start(&g_health_monitor_task));
    require_ok(hr_task_start(&g_consumer_task));
    require_ok(hr_task_start(&g_pulse_task));
    require_ok(hr_task_start(&g_producer_task));
    require_ok(hr_timer_start(&g_diagnostic_timer));

    board_uart_write_line(
        "Starting long-duration stress workload through SVC...");
    require_ok(hr_kernel_start());

    board_panic();
    return 0;
}
