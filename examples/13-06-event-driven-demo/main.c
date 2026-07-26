#include <stddef.h>
#include <stdint.h>

#include "board.h"
#include "hairevent/hairevent.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_time.h"

#define STACK_WORDS  256U
#define QUEUE_LENGTH 6U
#define SIGNAL_COUNT 64U

enum
{
    SIGNAL_START = HE_SIG_USER,
    SIGNAL_STOP,
    SIGNAL_HEARTBEAT,
    SIGNAL_STATUS
};

typedef struct
{
    he_event_t super;
    uint32_t heartbeat;
    hr_tick_t tick;
} status_event_t;

typedef union
{
    max_align_t alignment;
    unsigned char bytes[64];
} event_block_t;

typedef struct
{
    he_time_event_t *heartbeat_event;
    he_pubsub_t *bus;
    he_event_pool_t *pool;
    uint32_t heartbeat_count;
} controller_context_t;

static he_active_t g_controller;
static he_active_t g_observer;
static he_event_t *g_controller_queue[QUEUE_LENGTH];
static he_event_t *g_observer_queue[QUEUE_LENGTH];
static hr_stack_t g_controller_stack[STACK_WORDS];
static hr_stack_t g_observer_stack[STACK_WORDS];
static he_time_event_t g_heartbeat_event;
static he_pubsub_t g_bus;
static he_active_t *g_subscriber_slots[SIGNAL_COUNT * 2U];
static he_event_pool_t g_pool;
static event_block_t g_blocks[8];
static hr_task_t g_script;
static hr_stack_t g_script_stack[STACK_WORDS];
static he_event_t g_start_event;
static he_event_t g_stop_event;
static controller_context_t g_controller_context;

static he_state_result_t controller_active(he_state_machine_t *machine,
                                           const he_event_t *event);

static he_state_result_t controller_idle(he_state_machine_t *machine,
                                         const he_event_t *event)
{
    if (event->signal == HE_SIG_ENTRY)
    {
        board_led_off();
        board_uart_write_line("controller: IDLE");
        return HE_STATE_HANDLED;
    }
    if (event->signal == SIGNAL_START)
    {
        return he_state_transition(machine, controller_active);
    }
    return HE_STATE_IGNORED;
}

static he_state_result_t controller_active(he_state_machine_t *machine,
                                           const he_event_t *event)
{
    controller_context_t *context =
        (controller_context_t *)he_state_machine_context(machine);

    if (event->signal == HE_SIG_ENTRY)
    {
        board_led_on();
        board_uart_write_line("controller: ACTIVE, arm heartbeat time event");
        if (he_time_event_arm(context->heartbeat_event) != HR_OK)
        {
            board_panic();
        }
        return HE_STATE_HANDLED;
    }
    if (event->signal == HE_SIG_EXIT)
    {
        if (he_time_event_disarm(context->heartbeat_event) != HR_OK)
        {
            board_panic();
        }
        board_uart_write_line("controller: leave ACTIVE");
        return HE_STATE_HANDLED;
    }
    if (event->signal == SIGNAL_STOP)
    {
        return he_state_transition(machine, controller_idle);
    }
    if (event->signal == SIGNAL_HEARTBEAT)
    {
        status_event_t *status = (status_event_t *)he_event_new(
            context->pool,
            SIGNAL_STATUS,
            sizeof(status_event_t));
        size_t delivered = 0U;

        if (status == NULL)
        {
            board_panic();
        }
        context->heartbeat_count++;
        status->heartbeat = context->heartbeat_count;
        status->tick = hr_time_now();
        if (he_pubsub_publish(context->bus,
                              &status->super,
                              HR_NO_WAIT,
                              &delivered) != HR_OK ||
            (delivered != 1U))
        {
            board_panic();
        }
        return HE_STATE_HANDLED;
    }
    return HE_STATE_IGNORED;
}

static he_state_result_t observer_state(he_state_machine_t *machine,
                                        const he_event_t *event)
{
    (void)machine;

    if (event->signal == SIGNAL_STATUS)
    {
        const status_event_t *status = (const status_event_t *)event;
        board_uart_write_string("observer: heartbeat=");
        board_uart_write_u32(status->heartbeat);
        board_uart_write_string(" tick=");
        board_uart_write_u32(status->tick);
        board_uart_write_line("");
        return HE_STATE_HANDLED;
    }
    return (event->signal == HE_SIG_ENTRY) ?
           HE_STATE_HANDLED : HE_STATE_IGNORED;
}

static void script_task(void *argument)
{
    (void)argument;
    (void)hr_task_delay(200U);
    board_uart_write_line("script: START controller");
    if (he_active_post(&g_controller, &g_start_event, HR_WAIT_FOREVER) != HR_OK)
    {
        board_panic();
    }

    (void)hr_task_delay(1400U);
    board_uart_write_line("script: STOP controller");
    if (he_active_post(&g_controller, &g_stop_event, HR_WAIT_FOREVER) != HR_OK)
    {
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
    board_uart_write_line("hairtos Phase 13.6 - HairEvent integration demo");

    (void)he_event_init_static(&g_start_event,
                               SIGNAL_START,
                               sizeof(g_start_event));
    (void)he_event_init_static(&g_stop_event,
                               SIGNAL_STOP,
                               sizeof(g_stop_event));

    if ((he_event_pool_init(&g_pool,
                            g_blocks,
                            sizeof(g_blocks[0]),
                            8U) != HR_OK) ||
        (he_pubsub_init(&g_bus,
                        g_subscriber_slots,
                        SIGNAL_COUNT,
                        2U) != HR_OK) ||
        (hr_kernel_init() != HR_OK))
    {
        board_panic();
    }

    g_controller_context.heartbeat_event = &g_heartbeat_event;
    g_controller_context.bus = &g_bus;
    g_controller_context.pool = &g_pool;

    if ((he_active_create_static(&g_controller,
                                 "controller-AO",
                                 controller_idle,
                                 &g_controller_context,
                                 g_controller_queue,
                                 QUEUE_LENGTH,
                                 g_controller_stack,
                                 STACK_WORDS,
                                 2U) != HR_OK) ||
        (he_active_create_static(&g_observer,
                                 "observer-AO",
                                 observer_state,
                                 NULL,
                                 g_observer_queue,
                                 QUEUE_LENGTH,
                                 g_observer_stack,
                                 STACK_WORDS,
                                 3U) != HR_OK) ||
        (he_pubsub_subscribe(&g_bus,
                             SIGNAL_STATUS,
                             &g_observer) != HR_OK) ||
        (he_time_event_create_static(&g_heartbeat_event,
                                     "heartbeat-event",
                                     &g_controller,
                                     SIGNAL_HEARTBEAT,
                                     250U,
                                     true) != HR_OK) ||
        (hr_task_create_static(&g_script,
                               "event-script",
                               script_task,
                               NULL,
                               g_script_stack,
                               STACK_WORDS,
                               4U) != HR_OK) ||
        (hr_task_start(&g_script) != HR_OK))
    {
        board_panic();
    }

    status = hr_kernel_start();
    board_panic();
    return (int)status;
}
