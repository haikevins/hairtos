#include <stdint.h>

#include "board.h"
#include "hairevent/hairevent.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"

#define STACK_WORDS  224U
#define QUEUE_LENGTH 4U

enum { SIGNAL_TOGGLE = HE_SIG_USER };

static he_active_t g_switch;
static he_event_t *g_queue[QUEUE_LENGTH];
static hr_stack_t g_switch_stack[STACK_WORDS];
static hr_task_t g_controller;
static hr_stack_t g_controller_stack[STACK_WORDS];
static he_event_t g_toggle_event;

static he_state_result_t state_on(he_state_machine_t *machine,
                                  const he_event_t *event);

static he_state_result_t state_off(he_state_machine_t *machine,
                                   const he_event_t *event)
{
    switch (event->signal)
    {
        case HE_SIG_ENTRY:
            board_led_off();
            board_uart_write_line("state OFF: ENTRY");
            return HE_STATE_HANDLED;
        case HE_SIG_EXIT:
            board_uart_write_line("state OFF: EXIT");
            return HE_STATE_HANDLED;
        case SIGNAL_TOGGLE:
            return he_state_transition(machine, state_on);
        default:
            return HE_STATE_IGNORED;
    }
}

static he_state_result_t state_on(he_state_machine_t *machine,
                                  const he_event_t *event)
{
    switch (event->signal)
    {
        case HE_SIG_ENTRY:
            board_led_on();
            board_uart_write_line("state ON: ENTRY");
            return HE_STATE_HANDLED;
        case HE_SIG_EXIT:
            board_uart_write_line("state ON: EXIT");
            return HE_STATE_HANDLED;
        case SIGNAL_TOGGLE:
            return he_state_transition(machine, state_off);
        default:
            return HE_STATE_IGNORED;
    }
}

static void controller_task(void *argument)
{
    uint32_t index;
    (void)argument;

    for (index = 0U; index < 6U; index++)
    {
        (void)hr_task_delay(400U);
        if (he_active_post(&g_switch, &g_toggle_event, HR_WAIT_FOREVER) != HR_OK)
        {
            board_panic();
        }
    }

    board_uart_write_line("Flat state-machine ENTRY/EXIT transition demo: PASS");
    for (;;)
    {
        (void)hr_task_delay(1000U);
    }
}

int main(void)
{
    hr_status_t status;

    board_init();
    board_uart_write_line("HairRTOS Phase 13.3 - Flat state machine");
    (void)he_event_init_static(&g_toggle_event,
                               SIGNAL_TOGGLE,
                               sizeof(g_toggle_event));

    if (hr_kernel_init() != HR_OK)
    {
        board_panic();
    }

    if ((he_active_create_static(&g_switch,
                                 "switch-AO",
                                 state_off,
                                 NULL,
                                 g_queue,
                                 QUEUE_LENGTH,
                                 g_switch_stack,
                                 STACK_WORDS,
                                 2U) != HR_OK) ||
        (hr_task_create_static(&g_controller,
                               "toggle-controller",
                               controller_task,
                               NULL,
                               g_controller_stack,
                               STACK_WORDS,
                               3U) != HR_OK) ||
        (hr_task_start(&g_controller) != HR_OK))
    {
        board_panic();
    }

    status = hr_kernel_start();
    board_panic();
    return (int)status;
}
