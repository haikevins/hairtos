#include <stddef.h>
#include <stdint.h>

#include "board.h"
#include "haievent/haievent.h"
#include "hairtos/hairtos.h"
#include "hr_benchmark.h"
#include "hr_scheduler_internal.h"

#define BENCHMARK_SAMPLES          32U
#define TIMER_INTERVAL_SAMPLES     24U
#define TIMER_PERIOD_TICKS         10U
#define STARTUP_TASK_STACK_WORDS   128U
#define BENCHMARK_TASK_STACK_WORDS 320U
#define PEER_TASK_STACK_WORDS      160U
#define RECEIVER_STACK_WORDS       192U
#define EVENT_STACK_WORDS          224U
#define EVENT_QUEUE_CAPACITY       2U
#define PRIMITIVE_QUEUE_CAPACITY   4U

#define STARTUP_TASK_PRIORITY      0U
#define RECEIVER_TASK_PRIORITY     2U
#define EVENT_TASK_PRIORITY        3U
#define BENCHMARK_TASK_PRIORITY    4U

#define FLASH_BASE_ADDRESS         0x08000000UL
#define RAM_BASE_ADDRESS           0x20000000UL

enum
{
    BENCHMARK_EVENT_SIGNAL = HE_SIG_USER
};

extern unsigned char _sidata;
extern unsigned char _ebss;

static hr_task_t g_startup_task;
static hr_task_t g_benchmark_task;
static hr_task_t g_peer_task;
static hr_task_t g_receiver_task;
static hr_stack_t g_startup_stack[STARTUP_TASK_STACK_WORDS];
static hr_stack_t g_benchmark_stack[BENCHMARK_TASK_STACK_WORDS];
static hr_stack_t g_peer_stack[PEER_TASK_STACK_WORDS];
static hr_stack_t g_receiver_stack[RECEIVER_STACK_WORDS];

static hr_queue_t g_primitive_queue;
static uint32_t g_primitive_queue_storage[PRIMITIVE_QUEUE_CAPACITY];
static hr_queue_t g_wake_queue;
static uint32_t g_wake_queue_storage[1];
static hr_semaphore_t g_semaphore;
static hr_mutex_t g_mutex;
static hr_timer_t g_periodic_timer;

static he_active_t g_event_active;
static he_event_t *g_event_queue[EVENT_QUEUE_CAPACITY];
static hr_stack_t g_event_stack[EVENT_STACK_WORDS];
static he_event_t g_benchmark_event;

static volatile uint32_t g_startup_begin_cycles;
static volatile uint32_t g_startup_raw_cycles;
static volatile bool g_startup_captured;
static volatile uint32_t g_yield_request;
static volatile uint32_t g_yield_ack;
static volatile uint32_t g_wake_ack;
static volatile uint32_t g_event_request;
static volatile uint32_t g_event_ack;
static volatile uint32_t g_timer_callback_count;
static volatile bool g_timer_done;
static uint32_t g_timer_previous_cycles;
static uint32_t g_measurement_overhead_cycles;

static hr_benchmark_stats_t g_startup_stats;
static hr_benchmark_stats_t g_read_overhead_stats;
static hr_benchmark_stats_t g_critical_stats;
static hr_benchmark_stats_t g_scheduler_best_stats;
static hr_benchmark_stats_t g_scheduler_worst_stats;
static hr_benchmark_stats_t g_queue_stats;
static hr_benchmark_stats_t g_semaphore_stats;
static hr_benchmark_stats_t g_mutex_stats;
static hr_benchmark_stats_t g_timer_command_stats;
static hr_benchmark_stats_t g_yield_roundtrip_stats;
static hr_benchmark_stats_t g_queue_wake_stats;
static hr_benchmark_stats_t g_event_dispatch_stats;
static hr_benchmark_stats_t g_timer_interval_stats;
static hr_benchmark_stats_t g_timer_jitter_stats;

static hr_scheduler_t g_scheduler_best;
static hr_scheduler_t g_scheduler_worst;
static hr_ready_node_t g_scheduler_best_node;
static hr_ready_node_t g_scheduler_worst_node;

static void benchmark_require(bool condition)
{
    if (!condition)
    {
        board_panic();
    }
}

static void benchmark_require_status(hr_status_t status)
{
    benchmark_require(status == HR_OK);
}

static void benchmark_reset_all_stats(void)
{
    hr_benchmark_stats_reset(&g_startup_stats);
    hr_benchmark_stats_reset(&g_read_overhead_stats);
    hr_benchmark_stats_reset(&g_critical_stats);
    hr_benchmark_stats_reset(&g_scheduler_best_stats);
    hr_benchmark_stats_reset(&g_scheduler_worst_stats);
    hr_benchmark_stats_reset(&g_queue_stats);
    hr_benchmark_stats_reset(&g_semaphore_stats);
    hr_benchmark_stats_reset(&g_mutex_stats);
    hr_benchmark_stats_reset(&g_timer_command_stats);
    hr_benchmark_stats_reset(&g_yield_roundtrip_stats);
    hr_benchmark_stats_reset(&g_queue_wake_stats);
    hr_benchmark_stats_reset(&g_event_dispatch_stats);
    hr_benchmark_stats_reset(&g_timer_interval_stats);
    hr_benchmark_stats_reset(&g_timer_jitter_stats);
}

static void benchmark_record_adjusted(hr_benchmark_stats_t *stats,
                                      uint32_t start_cycles,
                                      uint32_t end_cycles)
{
    const uint32_t raw = hr_benchmark_elapsed_cycles(start_cycles, end_cycles);
    const uint32_t adjusted =
        hr_benchmark_adjust_cycles(raw, g_measurement_overhead_cycles);

    benchmark_require(hr_benchmark_stats_record(stats, adjusted));
}

static void benchmark_print_metric(const char *name,
                                   const hr_benchmark_stats_t *stats)
{
    const uint32_t mean_cycles = hr_benchmark_stats_mean(stats);

    board_uart_write_string(name);
    board_uart_write_char(',');
    board_uart_write_u32((uint32_t)hr_benchmark_stats_count(stats));
    board_uart_write_char(',');
    board_uart_write_u32(hr_benchmark_stats_min(stats));
    board_uart_write_char(',');
    board_uart_write_u32(hr_benchmark_stats_percentile(stats, 50U));
    board_uart_write_char(',');
    board_uart_write_u32(mean_cycles);
    board_uart_write_char(',');
    board_uart_write_u32(hr_benchmark_stats_percentile(stats, 95U));
    board_uart_write_char(',');
    board_uart_write_u32(hr_benchmark_stats_max(stats));
    board_uart_write_char(',');
    board_uart_write_u32(hr_benchmark_cycles_to_nanoseconds(
        mean_cycles,
        hr_benchmark_clock_frequency_hz()));
    board_uart_write_line("");
}

static void benchmark_print_stack(const char *name, const hr_task_t *task)
{
    board_uart_write_string("stack_free_words,");
    board_uart_write_string(name);
    board_uart_write_char(',');
    board_uart_write_u32((uint32_t)hr_task_get_stack_high_watermark(task));
    board_uart_write_char(',');
    board_uart_write_u32((uint32_t)hr_task_get_stack_words(task));
    board_uart_write_line("");
}

static void benchmark_print_report(void)
{
    const uintptr_t flash_bytes =
        (uintptr_t)&_sidata - (uintptr_t)FLASH_BASE_ADDRESS;
    const uintptr_t ram_static_bytes =
        (uintptr_t)&_ebss - (uintptr_t)RAM_BASE_ADDRESS;
    hr_task_t *const event_task = he_active_get_task(&g_event_active);

    board_uart_write_line("");
    board_uart_write_line("hairtos kernel benchmark report");
    board_uart_write_string("cpu,STM32F103C8T6 Cortex-M3,");
    board_uart_write_u32(board_get_core_clock_hz());
    board_uart_write_line(" Hz");
    board_uart_write_string("compiler,");
    board_uart_write_line(__VERSION__);
    board_uart_write_line("optimization,-Og");
    board_uart_write_string("tick_rate_hz,");
    board_uart_write_u32(HR_CFG_TICK_RATE_HZ);
    board_uart_write_line("");
    board_uart_write_string("preemption,");
    board_uart_write_u32(HR_CFG_PREEMPTION);
    board_uart_write_string(",time_slicing,");
    board_uart_write_u32(HR_CFG_TIME_SLICING);
    board_uart_write_line("");
    board_uart_write_string("measurement_overhead_cycles,");
    board_uart_write_u32(g_measurement_overhead_cycles);
    board_uart_write_line("");
    board_uart_write_line("metric,count,min,p50,mean,p95,max,mean_ns");

    benchmark_print_metric("svc_startup", &g_startup_stats);
    benchmark_print_metric("dwt_read_pair_raw", &g_read_overhead_stats);
    benchmark_print_metric("critical_enter_exit", &g_critical_stats);
    benchmark_print_metric("scheduler_select_priority0", &g_scheduler_best_stats);
    benchmark_print_metric("scheduler_select_priority6", &g_scheduler_worst_stats);
    benchmark_print_metric("queue_send_receive", &g_queue_stats);
    benchmark_print_metric("semaphore_take_give", &g_semaphore_stats);
    benchmark_print_metric("mutex_lock_unlock", &g_mutex_stats);
    benchmark_print_metric("timer_start_stop", &g_timer_command_stats);
    benchmark_print_metric("yield_roundtrip_two_switches", &g_yield_roundtrip_stats);
    benchmark_print_metric("queue_wake_preempt_roundtrip", &g_queue_wake_stats);
    benchmark_print_metric("event_post_dispatch_roundtrip", &g_event_dispatch_stats);
    benchmark_print_metric("timer_period_interval", &g_timer_interval_stats);
    benchmark_print_metric("timer_period_abs_jitter", &g_timer_jitter_stats);

    board_uart_write_string("flash_image_bytes,");
    board_uart_write_u32((uint32_t)flash_bytes);
    board_uart_write_line("");
    board_uart_write_string("static_ram_bytes,");
    board_uart_write_u32((uint32_t)ram_static_bytes);
    board_uart_write_line("");
    benchmark_print_stack("startup-probe", &g_startup_task);
    benchmark_print_stack("benchmark", &g_benchmark_task);
    benchmark_print_stack("peer", &g_peer_task);
    benchmark_print_stack("receiver", &g_receiver_task);
    benchmark_require(event_task != NULL);
    benchmark_print_stack("event-active", event_task);
    board_uart_write_line("gpio_marker,PB0 active-high around switch/wake samples");
    board_uart_write_line("Kernel benchmark: PASS");
}

static he_state_result_t benchmark_event_state(he_state_machine_t *machine,
                                               const he_event_t *event)
{
    (void)machine;

    if (event->signal == HE_SIG_ENTRY)
    {
        return HE_STATE_HANDLED;
    }

    if (event->signal == BENCHMARK_EVENT_SIGNAL)
    {
        g_event_ack = g_event_request;
        return HE_STATE_HANDLED;
    }

    return HE_STATE_IGNORED;
}

static void benchmark_startup_task(void *argument)
{
    (void)argument;

    g_startup_raw_cycles = hr_benchmark_elapsed_cycles(
        g_startup_begin_cycles,
        hr_benchmark_clock_now());
    g_startup_captured = true;
    benchmark_require_status(hr_task_suspend(hr_task_current()));
    board_panic();
}

static void benchmark_peer_task(void *argument)
{
    (void)argument;

    for (;;)
    {
        if (g_yield_ack != g_yield_request)
        {
            g_yield_ack = g_yield_request;
        }
        hr_task_yield();
    }
}

static void benchmark_receiver_task(void *argument)
{
    uint32_t sequence;

    (void)argument;
    for (;;)
    {
        benchmark_require_status(hr_queue_receive(&g_wake_queue,
                                                  &sequence,
                                                  HR_WAIT_FOREVER));
        g_wake_ack = sequence;
    }
}

static void benchmark_timer_callback(void *argument)
{
    const uint32_t now = hr_benchmark_clock_now();
    const uint32_t count = g_timer_callback_count;
    const uint32_t expected_cycles =
        (board_get_core_clock_hz() / HR_CFG_TICK_RATE_HZ) * TIMER_PERIOD_TICKS;

    (void)argument;

    if (count > 0U)
    {
        const uint32_t interval =
            hr_benchmark_elapsed_cycles(g_timer_previous_cycles, now);
        const uint32_t jitter = (interval >= expected_cycles) ?
            (interval - expected_cycles) : (expected_cycles - interval);

        benchmark_require(hr_benchmark_stats_record(&g_timer_interval_stats,
                                                    interval));
        benchmark_require(hr_benchmark_stats_record(&g_timer_jitter_stats,
                                                    jitter));
    }

    g_timer_previous_cycles = now;
    g_timer_callback_count = count + 1U;

    if (g_timer_callback_count >= (TIMER_INTERVAL_SAMPLES + 1U))
    {
        benchmark_require_status(hr_timer_stop(&g_periodic_timer));
        g_timer_done = true;
    }
}

static void benchmark_prepare_scheduler_sets(void)
{
    hr_scheduler_init(&g_scheduler_best);
    hr_scheduler_init(&g_scheduler_worst);
    hr_ready_node_init(&g_scheduler_best_node,
                       &g_scheduler_best_node,
                       0U);
    hr_ready_node_init(&g_scheduler_worst_node,
                       &g_scheduler_worst_node,
                       6U);
    benchmark_require_status(hr_scheduler_add_ready(&g_scheduler_best,
                                                    &g_scheduler_best_node));
    benchmark_require_status(hr_scheduler_add_ready(&g_scheduler_worst,
                                                    &g_scheduler_worst_node));
}

static void benchmark_measure_read_overhead(void)
{
    uint32_t index;

    for (index = 0U; index < BENCHMARK_SAMPLES; index++)
    {
        const uint32_t start = hr_benchmark_clock_now();
        const uint32_t end = hr_benchmark_clock_now();
        benchmark_require(hr_benchmark_stats_record(
            &g_read_overhead_stats,
            hr_benchmark_elapsed_cycles(start, end)));
    }

    g_measurement_overhead_cycles =
        hr_benchmark_stats_min(&g_read_overhead_stats);
}

static void benchmark_measure_critical_section(void)
{
    uint32_t index;

    for (index = 0U; index < BENCHMARK_SAMPLES; index++)
    {
        hr_irq_state_t irq_state;
        const uint32_t start = hr_benchmark_clock_now();
        irq_state = hr_critical_enter();
        hr_critical_exit(irq_state);
        benchmark_record_adjusted(&g_critical_stats,
                                  start,
                                  hr_benchmark_clock_now());
    }
}

static void benchmark_measure_scheduler(void)
{
    uint32_t index;

    for (index = 0U; index < BENCHMARK_SAMPLES; index++)
    {
        hr_ready_node_t *selected;
        uint32_t start = hr_benchmark_clock_now();
        selected = hr_scheduler_select_highest(&g_scheduler_best);
        benchmark_record_adjusted(&g_scheduler_best_stats,
                                  start,
                                  hr_benchmark_clock_now());
        benchmark_require((selected != NULL) && (selected->priority == 0U));

        start = hr_benchmark_clock_now();
        selected = hr_scheduler_select_highest(&g_scheduler_worst);
        benchmark_record_adjusted(&g_scheduler_worst_stats,
                                  start,
                                  hr_benchmark_clock_now());
        benchmark_require((selected != NULL) && (selected->priority == 6U));
    }
}

static void benchmark_measure_primitives(void)
{
    uint32_t index;
    uint32_t received;

    for (index = 0U; index < BENCHMARK_SAMPLES; index++)
    {
        const uint32_t value = index + 1U;
        uint32_t start = hr_benchmark_clock_now();
        benchmark_require_status(hr_queue_send(&g_primitive_queue,
                                               &value,
                                               HR_NO_WAIT));
        benchmark_require_status(hr_queue_receive(&g_primitive_queue,
                                                  &received,
                                                  HR_NO_WAIT));
        benchmark_record_adjusted(&g_queue_stats,
                                  start,
                                  hr_benchmark_clock_now());
        benchmark_require(received == value);

        start = hr_benchmark_clock_now();
        benchmark_require_status(hr_semaphore_take(&g_semaphore, HR_NO_WAIT));
        benchmark_require_status(hr_semaphore_give(&g_semaphore));
        benchmark_record_adjusted(&g_semaphore_stats,
                                  start,
                                  hr_benchmark_clock_now());

        start = hr_benchmark_clock_now();
        benchmark_require_status(hr_mutex_lock(&g_mutex, HR_NO_WAIT));
        benchmark_require_status(hr_mutex_unlock(&g_mutex));
        benchmark_record_adjusted(&g_mutex_stats,
                                  start,
                                  hr_benchmark_clock_now());

        start = hr_benchmark_clock_now();
        benchmark_require_status(hr_timer_start(&g_periodic_timer));
        benchmark_require_status(hr_timer_stop(&g_periodic_timer));
        benchmark_record_adjusted(&g_timer_command_stats,
                                  start,
                                  hr_benchmark_clock_now());
    }
}

static void benchmark_measure_yield_roundtrip(void)
{
    uint32_t index;

    for (index = 0U; index < BENCHMARK_SAMPLES; index++)
    {
        const uint32_t sequence = index + 1U;
        uint32_t start;
        uint32_t end;

        g_yield_request = sequence;
        hr_benchmark_gpio_mark_begin();
        start = hr_benchmark_clock_now();
        hr_task_yield();
        end = hr_benchmark_clock_now();
        hr_benchmark_gpio_mark_end();

        benchmark_require(g_yield_ack == sequence);
        benchmark_record_adjusted(&g_yield_roundtrip_stats, start, end);
    }
}

static void benchmark_measure_queue_wakeup(void)
{
    uint32_t index;

    for (index = 0U; index < BENCHMARK_SAMPLES; index++)
    {
        const uint32_t sequence = index + 1U;
        uint32_t start;
        uint32_t end;

        hr_benchmark_gpio_mark_begin();
        start = hr_benchmark_clock_now();
        benchmark_require_status(hr_queue_send(&g_wake_queue,
                                               &sequence,
                                               HR_NO_WAIT));
        end = hr_benchmark_clock_now();
        hr_benchmark_gpio_mark_end();

        benchmark_require(g_wake_ack == sequence);
        benchmark_record_adjusted(&g_queue_wake_stats, start, end);
    }
}

static void benchmark_measure_event_dispatch(void)
{
    uint32_t index;

    for (index = 0U; index < BENCHMARK_SAMPLES; index++)
    {
        const uint32_t sequence = index + 1U;
        uint32_t start;
        uint32_t end;

        g_event_request = sequence;
        hr_benchmark_gpio_mark_begin();
        start = hr_benchmark_clock_now();
        benchmark_require_status(he_active_post(&g_event_active,
                                                &g_benchmark_event,
                                                HR_NO_WAIT));
        end = hr_benchmark_clock_now();
        hr_benchmark_gpio_mark_end();

        benchmark_require(g_event_ack == sequence);
        benchmark_record_adjusted(&g_event_dispatch_stats, start, end);
    }
}

static void benchmark_measure_timer_jitter(void)
{
    g_timer_callback_count = 0U;
    g_timer_previous_cycles = 0U;
    g_timer_done = false;
    benchmark_require_status(hr_timer_start(&g_periodic_timer));

    while (!g_timer_done)
    {
        benchmark_require_status(hr_task_delay(1U));
    }

    benchmark_require(hr_benchmark_stats_count(&g_timer_interval_stats) ==
                      TIMER_INTERVAL_SAMPLES);
    benchmark_require(hr_benchmark_stats_count(&g_timer_jitter_stats) ==
                      TIMER_INTERVAL_SAMPLES);
}

static void benchmark_task_entry(void *argument)
{
    (void)argument;

    benchmark_require(g_startup_captured);
    benchmark_measure_read_overhead();
    benchmark_require(hr_benchmark_stats_record(
        &g_startup_stats,
        hr_benchmark_adjust_cycles(g_startup_raw_cycles,
                                   g_measurement_overhead_cycles)));
    benchmark_measure_critical_section();
    benchmark_measure_scheduler();
    benchmark_measure_primitives();
    benchmark_measure_yield_roundtrip();
    benchmark_measure_queue_wakeup();
    benchmark_measure_event_dispatch();
    benchmark_measure_timer_jitter();
    benchmark_print_report();

    for (;;)
    {
        board_led_toggle();
        benchmark_require_status(hr_task_delay(500U));
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("hairtos kernel benchmark");
    board_uart_write_line("Collecting DWT samples; UART output is deferred.");
    board_uart_write_line("PB0 is the active-high external timing marker.");

    if (!hr_benchmark_clock_init(board_get_core_clock_hz()))
    {
        board_uart_write_line("DWT CYCCNT unavailable");
        board_panic();
    }
    hr_benchmark_gpio_init();
    benchmark_reset_all_stats();
    benchmark_prepare_scheduler_sets();

    status = hr_kernel_init();
    if (status != HR_OK)
    {
        board_panic();
    }

    benchmark_require_status(hr_queue_create_static(
        &g_primitive_queue,
        g_primitive_queue_storage,
        sizeof(g_primitive_queue_storage[0]),
        PRIMITIVE_QUEUE_CAPACITY));
    benchmark_require_status(hr_queue_create_static(
        &g_wake_queue,
        g_wake_queue_storage,
        sizeof(g_wake_queue_storage[0]),
        1U));
    benchmark_require_status(hr_semaphore_create_binary(&g_semaphore, true));
    benchmark_require_status(hr_mutex_create(&g_mutex));
    benchmark_require_status(he_event_init_static(&g_benchmark_event,
                                                  BENCHMARK_EVENT_SIGNAL,
                                                  sizeof(g_benchmark_event)));
    benchmark_require_status(hr_timer_create_static(&g_periodic_timer,
                                                    "benchmark-periodic",
                                                    TIMER_PERIOD_TICKS,
                                                    true,
                                                    benchmark_timer_callback,
                                                    NULL));
    benchmark_require_status(he_active_create_static(
        &g_event_active,
        "benchmark-event-active",
        benchmark_event_state,
        NULL,
        g_event_queue,
        EVENT_QUEUE_CAPACITY,
        g_event_stack,
        EVENT_STACK_WORDS,
        EVENT_TASK_PRIORITY));

    benchmark_require_status(hr_task_create_static(&g_startup_task,
                                                   "startup-probe",
                                                   benchmark_startup_task,
                                                   NULL,
                                                   g_startup_stack,
                                                   STARTUP_TASK_STACK_WORDS,
                                                   STARTUP_TASK_PRIORITY));
    benchmark_require_status(hr_task_create_static(&g_receiver_task,
                                                   "benchmark-receiver",
                                                   benchmark_receiver_task,
                                                   NULL,
                                                   g_receiver_stack,
                                                   RECEIVER_STACK_WORDS,
                                                   RECEIVER_TASK_PRIORITY));
    benchmark_require_status(hr_task_create_static(&g_benchmark_task,
                                                   "benchmark-runner",
                                                   benchmark_task_entry,
                                                   NULL,
                                                   g_benchmark_stack,
                                                   BENCHMARK_TASK_STACK_WORDS,
                                                   BENCHMARK_TASK_PRIORITY));
    benchmark_require_status(hr_task_create_static(&g_peer_task,
                                                   "benchmark-peer",
                                                   benchmark_peer_task,
                                                   NULL,
                                                   g_peer_stack,
                                                   PEER_TASK_STACK_WORDS,
                                                   BENCHMARK_TASK_PRIORITY));
    benchmark_require_status(hr_task_start(&g_startup_task));
    benchmark_require_status(hr_task_start(&g_receiver_task));
    benchmark_require_status(hr_task_start(&g_benchmark_task));
    benchmark_require_status(hr_task_start(&g_peer_task));

    g_startup_captured = false;
    g_startup_begin_cycles = hr_benchmark_clock_now();
    status = hr_kernel_start();
    board_panic();
    return (int)status;
}
