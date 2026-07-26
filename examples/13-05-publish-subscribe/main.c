#include <stddef.h>
#include <stdint.h>

#include "board.h"
#include "haievent/haievent.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"

#define STACK_WORDS  224U
#define QUEUE_LENGTH 4U
#define SIGNAL_COUNT 64U

enum { SIGNAL_TELEMETRY = HE_SIG_USER };

typedef struct
{
    he_event_t super;
    uint32_t sequence;
} telemetry_event_t;

typedef union
{
    max_align_t alignment;
    unsigned char bytes[64];
} event_block_t;

typedef struct
{
    const char *name;
    uint32_t count;
} subscriber_context_t;

static he_event_pool_t g_pool;
static event_block_t g_blocks[6];
static he_pubsub_t g_bus;
static he_active_t *g_subscriber_slots[SIGNAL_COUNT * 2U];
static he_active_t g_logger;
static he_active_t g_display;
static he_event_t *g_logger_queue[QUEUE_LENGTH];
static he_event_t *g_display_queue[QUEUE_LENGTH];
static hr_stack_t g_logger_stack[STACK_WORDS];
static hr_stack_t g_display_stack[STACK_WORDS];
static hr_task_t g_publisher;
static hr_stack_t g_publisher_stack[STACK_WORDS];
static subscriber_context_t g_logger_context = {"logger", 0U};
static subscriber_context_t g_display_context = {"display", 0U};

static he_state_result_t subscriber_state(he_state_machine_t *machine,
                                          const he_event_t *event)
{
    subscriber_context_t *context =
        (subscriber_context_t *)he_state_machine_context(machine);

    if (event->signal == SIGNAL_TELEMETRY)
    {
        const telemetry_event_t *telemetry =
            (const telemetry_event_t *)event;
        context->count++;
        board_uart_write_string(context->name);
        board_uart_write_string(": telemetry sequence=");
        board_uart_write_u32(telemetry->sequence);
        board_uart_write_string(" count=");
        board_uart_write_u32(context->count);
        board_uart_write_line("");
        return HE_STATE_HANDLED;
    }

    return (event->signal == HE_SIG_ENTRY) ?
           HE_STATE_HANDLED : HE_STATE_IGNORED;
}

static void publisher_task(void *argument)
{
    uint32_t sequence = 1U;
    (void)argument;

    for (;;)
    {
        telemetry_event_t *event = (telemetry_event_t *)he_event_new(
            &g_pool,
            SIGNAL_TELEMETRY,
            sizeof(telemetry_event_t));
        size_t delivered = 0U;

        if (event == NULL)
        {
            board_panic();
        }
        event->sequence = sequence;

        if (he_pubsub_publish(&g_bus,
                              &event->super,
                              HR_WAIT_FOREVER,
                              &delivered) != HR_OK)
        {
            board_panic();
        }

        board_uart_write_string("publisher: delivered=");
        board_uart_write_u32((uint32_t)delivered);
        board_uart_write_string(" sequence=");
        board_uart_write_u32(sequence);
        board_uart_write_line("");

        sequence++;
        (void)hr_task_delay(500U);
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("hairtos Phase 13.5 - Publish/Subscribe");

    if ((he_event_pool_init(&g_pool,
                            g_blocks,
                            sizeof(g_blocks[0]),
                            6U) != HR_OK) ||
        (he_pubsub_init(&g_bus,
                        g_subscriber_slots,
                        SIGNAL_COUNT,
                        2U) != HR_OK) ||
        (hr_kernel_init() != HR_OK))
    {
        board_panic();
    }

    if ((he_active_create_static(&g_logger,
                                 "logger-AO",
                                 subscriber_state,
                                 &g_logger_context,
                                 g_logger_queue,
                                 QUEUE_LENGTH,
                                 g_logger_stack,
                                 STACK_WORDS,
                                 2U) != HR_OK) ||
        (he_active_create_static(&g_display,
                                 "display-AO",
                                 subscriber_state,
                                 &g_display_context,
                                 g_display_queue,
                                 QUEUE_LENGTH,
                                 g_display_stack,
                                 STACK_WORDS,
                                 3U) != HR_OK) ||
        (he_pubsub_subscribe(&g_bus,
                             SIGNAL_TELEMETRY,
                             &g_logger) != HR_OK) ||
        (he_pubsub_subscribe(&g_bus,
                             SIGNAL_TELEMETRY,
                             &g_display) != HR_OK) ||
        (hr_task_create_static(&g_publisher,
                               "publisher",
                               publisher_task,
                               NULL,
                               g_publisher_stack,
                               STACK_WORDS,
                               4U) != HR_OK) ||
        (hr_task_start(&g_publisher) != HR_OK))
    {
        board_panic();
    }

    status = hr_kernel_start();
    board_panic();
    return (int)status;
}
