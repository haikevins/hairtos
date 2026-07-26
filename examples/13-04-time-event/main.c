#include <stdint.h>

#include "board.h"
#include "haievent/haievent.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_time.h"

#define STACK_WORDS  224U
#define QUEUE_LENGTH 6U

enum { SIGNAL_TICK = HE_SIG_USER };

static he_active_t g_blinker;
static he_event_t *g_queue[QUEUE_LENGTH];
static hr_stack_t g_stack[STACK_WORDS];
static he_time_event_t g_tick_event;
static uint32_t g_count;

static he_state_result_t blinker_state(he_state_machine_t *machine,
                                       const he_event_t *event)
{
    (void)machine;

    if (event->signal == HE_SIG_ENTRY)
    {
        board_uart_write_line("blinker AO: waiting for periodic time events");
        return HE_STATE_HANDLED;
    }

    if (event->signal == SIGNAL_TICK)
    {
        g_count++;
        board_led_toggle();
        board_uart_write_string("time-event count=");
        board_uart_write_u32(g_count);
        board_uart_write_string(" tick=");
        board_uart_write_u32(hr_time_now());
        board_uart_write_line("");

        if (g_count == 6U)
        {
            if (he_time_event_disarm(&g_tick_event) != HR_OK)
            {
                board_panic();
            }
            board_uart_write_line("Time event: PASS");
        }
        return HE_STATE_HANDLED;
    }

    return HE_STATE_IGNORED;
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("hairtos time event");
    board_uart_write_line("Timer service posts events; the AO dispatches them in task context.");

    if (hr_kernel_init() != HR_OK)
    {
        board_panic();
    }

    if ((he_active_create_static(&g_blinker,
                                 "blinker-AO",
                                 blinker_state,
                                 NULL,
                                 g_queue,
                                 QUEUE_LENGTH,
                                 g_stack,
                                 STACK_WORDS,
                                 2U) != HR_OK) ||
        (he_time_event_create_static(&g_tick_event,
                                     "blink-time-event",
                                     &g_blinker,
                                     SIGNAL_TICK,
                                     250U,
                                     true) != HR_OK) ||
        (he_time_event_arm(&g_tick_event) != HR_OK))
    {
        board_panic();
    }

    status = hr_kernel_start();
    board_panic();
    return (int)status;
}
