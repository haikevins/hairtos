#include <stdint.h>

#include "board.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_queue.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_time.h"
#include "hr_port.h"

#define PHASE9_CONSUMER_PRIORITY       1U
#define PHASE9_PRODUCER_PRIORITY       3U
#define PHASE9_STACK_WORDS             224U
#define PHASE9_QUEUE_CAPACITY          2U
#define PHASE9_CONSUMER_DELAY_TICKS    200U
#define PHASE9_SEND_TIMEOUT_TICKS      100U

typedef struct
{
    uint32_t sequence;
    hr_tick_t produced_at;
} phase9_message_t;

static hr_queue_t g_message_queue;
static phase9_message_t g_queue_storage[PHASE9_QUEUE_CAPACITY];

static hr_task_t g_consumer_task;
static hr_task_t g_producer_task;
static hr_stack_t g_consumer_stack[PHASE9_STACK_WORDS];
static hr_stack_t g_producer_stack[PHASE9_STACK_WORDS];

static volatile uint32_t g_sent_count;
static volatile uint32_t g_timeout_count;

static void phase9_verify_context(const hr_task_t *expected)
{
    if ((hr_task_current() != expected) || !hr_port_thread_uses_psp())
    {
        board_uart_write_line("ERROR: invalid Phase 9 task context.");
        board_panic();
    }
}

static void consumer_task(void *argument)
{
    uint32_t previous_sequence = 0U;

    (void)argument;

    for (;;)
    {
        phase9_message_t message;
        hr_status_t status;

        phase9_verify_context(&g_consumer_task);
        status = hr_queue_receive(&g_message_queue,
                                  &message,
                                  HR_WAIT_FOREVER);
        if (status != HR_OK)
        {
            board_uart_write_line("ERROR: blocking queue receive failed.");
            board_panic();
        }

        if (message.sequence <= previous_sequence)
        {
            board_uart_write_line("ERROR: queue FIFO sequence violated.");
            board_panic();
        }
        previous_sequence = message.sequence;

        board_led_toggle();
        board_uart_write_string("consumer received seq=");
        board_uart_write_u32(message.sequence);
        board_uart_write_string(" produced_at=");
        board_uart_write_u32(message.produced_at);
        board_uart_write_string(" now=");
        board_uart_write_u32(hr_time_now());
        board_uart_write_string(" queued=");
        board_uart_write_u32((uint32_t)hr_queue_get_count(&g_message_queue));
        board_uart_write_string(" send_timeouts=");
        board_uart_write_u32(g_timeout_count);
        board_uart_write_line("");

        if (hr_task_delay(PHASE9_CONSUMER_DELAY_TICKS) != HR_OK)
        {
            board_uart_write_line("ERROR: consumer delay failed.");
            board_panic();
        }
    }
}

static void producer_task(void *argument)
{
    uint32_t sequence = 1U;

    (void)argument;

    for (;;)
    {
        phase9_message_t message;
        hr_status_t status;

        phase9_verify_context(&g_producer_task);
        message.sequence = sequence;
        message.produced_at = hr_time_now();

        status = hr_queue_send(&g_message_queue,
                               &message,
                               PHASE9_SEND_TIMEOUT_TICKS);
        if (status == HR_OK)
        {
            g_sent_count++;
        }
        else if (status == HR_ERROR_TIMEOUT)
        {
            g_timeout_count++;
        }
        else
        {
            board_uart_write_line("ERROR: blocking queue send failed.");
            board_panic();
        }

        sequence++;
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("hairtos Phase 9");
    board_uart_write_line("Static FIFO queue with blocking send/receive and timeout.");
    board_uart_write_line("The high-priority consumer blocks; producer wake-up preempts immediately.");

    status = hr_queue_create_static(&g_message_queue,
                                    g_queue_storage,
                                    sizeof(g_queue_storage[0]),
                                    PHASE9_QUEUE_CAPACITY);
    if (status != HR_OK)
    {
        board_uart_write_line("Queue creation failed.");
        board_panic();
    }

    status = hr_kernel_init();
    if (status != HR_OK)
    {
        board_uart_write_line("Kernel initialization failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_consumer_task,
                                   "consumer",
                                   consumer_task,
                                   NULL,
                                   g_consumer_stack,
                                   PHASE9_STACK_WORDS,
                                   PHASE9_CONSUMER_PRIORITY);
    if (status != HR_OK)
    {
        board_uart_write_line("Consumer task creation failed.");
        board_panic();
    }

    status = hr_task_create_static(&g_producer_task,
                                   "producer",
                                   producer_task,
                                   NULL,
                                   g_producer_stack,
                                   PHASE9_STACK_WORDS,
                                   PHASE9_PRODUCER_PRIORITY);
    if (status != HR_OK)
    {
        board_uart_write_line("Producer task creation failed.");
        board_panic();
    }

    if ((hr_task_start(&g_producer_task) != HR_OK) ||
        (hr_task_start(&g_consumer_task) != HR_OK))
    {
        board_uart_write_line("Task registration failed.");
        board_panic();
    }

    board_uart_write_line("Starting Phase 9 scheduler through SVC...");
    status = hr_kernel_start();

    board_uart_write_string("ERROR: hr_kernel_start returned status=");
    board_uart_write_u32((uint32_t)status);
    board_uart_write_line("");
    board_panic();
    return (int)status;
}
