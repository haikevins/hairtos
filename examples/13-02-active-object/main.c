#include <stdint.h>

#include "board.h"
#include "hairevent/hairevent.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"

#define STACK_WORDS  224U
#define QUEUE_LENGTH 4U

enum
{
    SIGNAL_PING = HE_SIG_USER,
    SIGNAL_PONG
};

typedef struct
{
    const char *name;
    he_active_t *peer;
    he_event_t *reply;
    uint32_t count;
} actor_context_t;

static he_active_t g_ping;
static he_active_t g_pong;
static he_event_t *g_ping_queue[QUEUE_LENGTH];
static he_event_t *g_pong_queue[QUEUE_LENGTH];
static hr_stack_t g_ping_stack[STACK_WORDS];
static hr_stack_t g_pong_stack[STACK_WORDS];
static hr_task_t g_starter;
static hr_stack_t g_starter_stack[STACK_WORDS];
static he_event_t g_ping_event;
static he_event_t g_pong_event;
static actor_context_t g_ping_context;
static actor_context_t g_pong_context;

static he_state_result_t actor_state(he_state_machine_t *machine,
                                     const he_event_t *event)
{
    actor_context_t *context =
        (actor_context_t *)he_state_machine_context(machine);

    if (event->signal == HE_SIG_ENTRY)
    {
        board_uart_write_string(context->name);
        board_uart_write_line(": Active Object started");
        return HE_STATE_HANDLED;
    }

    if ((event->signal == SIGNAL_PING) || (event->signal == SIGNAL_PONG))
    {
        context->count++;
        board_uart_write_string(context->name);
        board_uart_write_string(": handled event count=");
        board_uart_write_u32(context->count);
        board_uart_write_line("");
        if (he_active_post(context->peer, context->reply, HR_NO_WAIT) != HR_OK)
        {
            board_panic();
        }
        return HE_STATE_HANDLED;
    }

    return HE_STATE_IGNORED;
}

static void starter_task(void *argument)
{
    (void)argument;
    (void)hr_task_delay(100U);
    board_uart_write_line("starter: post first PING");
    if (he_active_post(&g_ping, &g_ping_event, HR_NO_WAIT) != HR_OK)
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
    board_uart_write_line("hairtos Phase 13.2 - Active Objects");

    (void)he_event_init_static(&g_ping_event,
                               SIGNAL_PING,
                               sizeof(g_ping_event));
    (void)he_event_init_static(&g_pong_event,
                               SIGNAL_PONG,
                               sizeof(g_pong_event));

    status = hr_kernel_init();
    if (status != HR_OK)
    {
        board_panic();
    }

    g_ping_context.name = "ping-AO";
    g_ping_context.peer = &g_pong;
    g_ping_context.reply = &g_pong_event;
    g_pong_context.name = "pong-AO";
    g_pong_context.peer = &g_ping;
    g_pong_context.reply = &g_ping_event;

    if ((he_active_create_static(&g_ping,
                                 "ping-AO",
                                 actor_state,
                                 &g_ping_context,
                                 g_ping_queue,
                                 QUEUE_LENGTH,
                                 g_ping_stack,
                                 STACK_WORDS,
                                 2U) != HR_OK) ||
        (he_active_create_static(&g_pong,
                                 "pong-AO",
                                 actor_state,
                                 &g_pong_context,
                                 g_pong_queue,
                                 QUEUE_LENGTH,
                                 g_pong_stack,
                                 STACK_WORDS,
                                 3U) != HR_OK) ||
        (hr_task_create_static(&g_starter,
                               "starter",
                               starter_task,
                               NULL,
                               g_starter_stack,
                               STACK_WORDS,
                               4U) != HR_OK) ||
        (hr_task_start(&g_starter) != HR_OK))
    {
        board_panic();
    }

    status = hr_kernel_start();
    board_panic();
    return (int)status;
}
