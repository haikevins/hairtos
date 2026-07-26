#include <stddef.h>
#include <stdint.h>

#include "hairtos_config.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hr_kernel_internal.h"
#include "hr_list_internal.h"
#include "hr_port.h"
#include "hr_scheduler_internal.h"
#include "hr_task_internal.h"
#include "hr_timeout_internal.h"

#define HR_SWITCH_REASON_NONE        UINT32_C(0)
#define HR_SWITCH_REASON_YIELD       (UINT32_C(1) << 0U)
#define HR_SWITCH_REASON_BLOCK       (UINT32_C(1) << 1U)
#define HR_SWITCH_REASON_PREEMPT     (UINT32_C(1) << 2U)
#define HR_SWITCH_REASON_TIME_SLICE  (UINT32_C(1) << 3U)

static hr_kernel_state_t g_kernel_state = HR_KERNEL_STATE_RESET;
static hr_scheduler_t g_scheduler;
static hr_list_t g_all_tasks;
static hr_timeout_list_t g_timeout_list;
static hr_task_t *g_current_task;
static size_t g_task_count;
static volatile hr_tick_t g_kernel_tick;
static volatile uint32_t g_switch_reasons;

static hr_task_t g_idle_task;
static hr_stack_t g_idle_stack[HR_CFG_IDLE_STACK_WORDS];

hr_task_control_block_t *g_hr_current_task_control_block;

static void hr_kernel_panic(void)
{
    g_kernel_state = HR_KERNEL_STATE_PANIC;
}

static void hr_idle_task(void *argument)
{
    (void)argument;

    for (;;)
    {
        hr_port_wait_for_interrupt();
    }
}

static void hr_kernel_mark_switch_reason(uint32_t reason)
{
    g_switch_reasons |= reason;
}

static void hr_kernel_reload_time_slice(hr_task_control_block_t *control_block)
{
    if (control_block != NULL)
    {
        control_block->time_slice_remaining = HR_CFG_TIME_SLICE_TICKS;
    }
}

hr_status_t hr_kernel_init(void)
{
    hr_status_t status;

    if (g_kernel_state != HR_KERNEL_STATE_RESET)
    {
        return HR_ERROR_INVALID_STATE;
    }

    hr_scheduler_init(&g_scheduler);
    hr_list_init(&g_all_tasks);
    hr_timeout_list_init(&g_timeout_list, 0U);
    g_current_task = NULL;
    g_hr_current_task_control_block = NULL;
    g_task_count = 0U;
    g_kernel_tick = 0U;
    g_switch_reasons = HR_SWITCH_REASON_NONE;
    g_kernel_state = HR_KERNEL_STATE_INITIALIZED;

    status = hr_task_create_static(&g_idle_task,
                                   "idle",
                                   hr_idle_task,
                                   NULL,
                                   g_idle_stack,
                                   HR_CFG_IDLE_STACK_WORDS,
                                   (hr_priority_t)HR_CFG_IDLE_PRIORITY);
    if (status != HR_OK)
    {
        hr_kernel_panic();
        return status;
    }

    status = hr_task_start(&g_idle_task);
    if (status != HR_OK)
    {
        hr_kernel_panic();
        return status;
    }

    return HR_OK;
}

hr_status_t hr_kernel_register_task(hr_task_t *task)
{
    hr_task_control_block_t *control_block;
    hr_status_t status;

    if ((g_kernel_state != HR_KERNEL_STATE_INITIALIZED) || !hr_task_is_valid(task))
    {
        return HR_ERROR_INVALID_STATE;
    }

    if (g_task_count >= (size_t)HR_CFG_MAX_TASKS)
    {
        return HR_ERROR_NO_MEMORY;
    }

    control_block = hr_task_control_block(task);
    if ((task != &g_idle_task) &&
        (control_block->effective_priority == (hr_priority_t)HR_CFG_IDLE_PRIORITY))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if ((control_block->state != HR_TASK_STATE_CREATED) ||
        hr_list_node_is_linked(&control_block->ready_node.node) ||
        hr_list_node_is_linked(&control_block->all_task_node))
    {
        return HR_ERROR_INVALID_STATE;
    }

    status = hr_scheduler_add_ready(&g_scheduler, &control_block->ready_node);
    if (status != HR_OK)
    {
        return status;
    }

    status = hr_list_push_back(&g_all_tasks, &control_block->all_task_node);
    if (status != HR_OK)
    {
        (void)hr_scheduler_remove_ready(&g_scheduler, &control_block->ready_node);
        return status;
    }

    status = hr_task_transition_state(task,
                                      HR_TASK_STATE_CREATED,
                                      HR_TASK_STATE_READY);
    if (status != HR_OK)
    {
        (void)hr_list_remove(&control_block->all_task_node);
        (void)hr_scheduler_remove_ready(&g_scheduler, &control_block->ready_node);
        return status;
    }

    hr_kernel_reload_time_slice(control_block);
    g_task_count++;
    return HR_OK;
}

hr_status_t hr_kernel_prepare_start(void)
{
    hr_ready_node_t *ready_node;
    hr_task_t *selected_task;
    hr_task_control_block_t *control_block;

    if (g_kernel_state != HR_KERNEL_STATE_INITIALIZED)
    {
        return HR_ERROR_INVALID_STATE;
    }

    ready_node = hr_scheduler_select_highest(&g_scheduler);
    if (ready_node == NULL)
    {
        return HR_ERROR_INTERNAL;
    }

    selected_task = (hr_task_t *)hr_list_node_owner(&ready_node->node);
    if (!hr_task_is_valid(selected_task))
    {
        return HR_ERROR_INTERNAL;
    }

    control_block = hr_task_control_block(selected_task);
    if (control_block->state != HR_TASK_STATE_READY)
    {
        return HR_ERROR_INVALID_STATE;
    }

    if (hr_task_transition_state(selected_task,
                                 HR_TASK_STATE_READY,
                                 HR_TASK_STATE_RUNNING) != HR_OK)
    {
        return HR_ERROR_INVALID_STATE;
    }

    hr_kernel_reload_time_slice(control_block);
    g_current_task = selected_task;
    g_hr_current_task_control_block = control_block;
    g_kernel_state = HR_KERNEL_STATE_RUNNING;
    return HR_OK;
}

void hr_kernel_request_yield(void)
{
    hr_irq_state_t irq_state;
    bool request_switch = false;

    irq_state = hr_port_enter_critical();
    if ((g_kernel_state == HR_KERNEL_STATE_RUNNING) &&
        (g_current_task != NULL) &&
        (hr_task_control_block(g_current_task)->state == HR_TASK_STATE_RUNNING))
    {
        hr_kernel_mark_switch_reason(HR_SWITCH_REASON_YIELD);
        request_switch = true;
    }
    hr_port_exit_critical(irq_state);

    if (request_switch)
    {
        hr_port_request_context_switch();
    }
}

hr_status_t hr_kernel_delay_current(hr_tick_t delay_ticks)
{
    hr_task_control_block_t *control_block;
    hr_status_t status;

    if ((g_kernel_state != HR_KERNEL_STATE_RUNNING) ||
        (g_current_task == NULL) ||
        (g_current_task == &g_idle_task) ||
        (delay_ticks == 0U) ||
        (delay_ticks == HR_WAIT_FOREVER))
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = hr_task_control_block(g_current_task);
    if ((control_block->state != HR_TASK_STATE_RUNNING) ||
        !hr_list_node_is_linked(&control_block->ready_node.node) ||
        hr_list_node_is_linked(&control_block->timeout_node.node))
    {
        return HR_ERROR_INVALID_STATE;
    }

    status = hr_scheduler_remove_ready(&g_scheduler, &control_block->ready_node);
    if (status != HR_OK)
    {
        return status;
    }

    status = hr_timeout_list_insert(&g_timeout_list,
                                    &control_block->timeout_node,
                                    delay_ticks);
    if (status != HR_OK)
    {
        (void)hr_scheduler_add_ready(&g_scheduler, &control_block->ready_node);
        return status;
    }

    status = hr_task_transition_state(g_current_task,
                                      HR_TASK_STATE_RUNNING,
                                      HR_TASK_STATE_BLOCKED);
    if (status != HR_OK)
    {
        (void)hr_timeout_list_remove(&g_timeout_list, &control_block->timeout_node);
        (void)hr_scheduler_add_ready(&g_scheduler, &control_block->ready_node);
        return status;
    }

    control_block->wake_tick = g_kernel_tick + delay_ticks;
    control_block->waiting_object = &g_timeout_list;
    control_block->blocked_wait_list = NULL;
    control_block->wait_buffer = NULL;
    control_block->wait_result = HR_OK;
    control_block->wait_kind = HR_TASK_WAIT_DELAY;
    hr_kernel_reload_time_slice(control_block);
    hr_kernel_mark_switch_reason(HR_SWITCH_REASON_BLOCK);
    return HR_OK;
}

hr_status_t hr_kernel_suspend_task(hr_task_t *task, bool *switch_required)
{
    hr_task_control_block_t *control_block;
    hr_status_t status;

    if (switch_required != NULL)
    {
        *switch_required = false;
    }

    if ((g_kernel_state != HR_KERNEL_STATE_RUNNING) ||
        !hr_task_is_valid(task) || (task == &g_idle_task))
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = hr_task_control_block(task);
    switch (control_block->state)
    {
        case HR_TASK_STATE_READY:
            if (!hr_list_node_is_linked(&control_block->ready_node.node))
            {
                return HR_ERROR_INTERNAL;
            }

            status = hr_scheduler_remove_ready(&g_scheduler,
                                               &control_block->ready_node);
            if (status != HR_OK)
            {
                return status;
            }

            status = hr_task_transition_state(task,
                                              HR_TASK_STATE_READY,
                                              HR_TASK_STATE_SUSPENDED);
            if (status != HR_OK)
            {
                (void)hr_scheduler_add_ready(&g_scheduler,
                                             &control_block->ready_node);
                return status;
            }
            control_block->suspended_resume_state = HR_TASK_STATE_READY;
            return HR_OK;

        case HR_TASK_STATE_RUNNING:
            if ((task != g_current_task) ||
                !hr_list_node_is_linked(&control_block->ready_node.node))
            {
                return HR_ERROR_INVALID_STATE;
            }

            status = hr_scheduler_remove_ready(&g_scheduler,
                                               &control_block->ready_node);
            if (status != HR_OK)
            {
                return status;
            }

            status = hr_task_transition_state(task,
                                              HR_TASK_STATE_RUNNING,
                                              HR_TASK_STATE_SUSPENDED);
            if (status != HR_OK)
            {
                (void)hr_scheduler_add_ready(&g_scheduler,
                                             &control_block->ready_node);
                return status;
            }

            control_block->suspended_resume_state = HR_TASK_STATE_READY;
            hr_kernel_reload_time_slice(control_block);
            hr_kernel_mark_switch_reason(HR_SWITCH_REASON_BLOCK);
            if (switch_required != NULL)
            {
                *switch_required = true;
            }
            return HR_OK;

        case HR_TASK_STATE_BLOCKED:
            if ((control_block->wait_kind == HR_TASK_WAIT_NONE) ||
                (control_block->waiting_object == NULL) ||
                hr_list_node_is_linked(&control_block->ready_node.node))
            {
                return HR_ERROR_INTERNAL;
            }

            status = hr_task_transition_state(task,
                                              HR_TASK_STATE_BLOCKED,
                                              HR_TASK_STATE_SUSPENDED);
            if (status != HR_OK)
            {
                return status;
            }
            control_block->suspended_resume_state = HR_TASK_STATE_BLOCKED;
            return HR_OK;

        default:
            return HR_ERROR_INVALID_STATE;
    }
}

hr_status_t hr_kernel_resume_task(hr_task_t *task, bool *switch_required)
{
    hr_task_control_block_t *control_block;
    hr_task_control_block_t *current_control_block;
    hr_task_state_t resume_state;
    hr_status_t status;

    if (switch_required != NULL)
    {
        *switch_required = false;
    }

    if ((g_kernel_state != HR_KERNEL_STATE_RUNNING) ||
        !hr_task_is_valid(task) || (task == &g_idle_task))
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = hr_task_control_block(task);
    if (control_block->state != HR_TASK_STATE_SUSPENDED)
    {
        return HR_ERROR_INVALID_STATE;
    }

    resume_state = control_block->suspended_resume_state;
    if (resume_state == HR_TASK_STATE_BLOCKED)
    {
        status = hr_task_transition_state(task,
                                          HR_TASK_STATE_SUSPENDED,
                                          HR_TASK_STATE_BLOCKED);
        if (status == HR_OK)
        {
            control_block->suspended_resume_state = HR_TASK_STATE_INVALID;
        }
        return status;
    }

    if (resume_state != HR_TASK_STATE_READY)
    {
        return HR_ERROR_INTERNAL;
    }

    status = hr_scheduler_add_ready(&g_scheduler, &control_block->ready_node);
    if (status != HR_OK)
    {
        return status;
    }

    status = hr_task_transition_state(task,
                                      HR_TASK_STATE_SUSPENDED,
                                      HR_TASK_STATE_READY);
    if (status != HR_OK)
    {
        (void)hr_scheduler_remove_ready(&g_scheduler,
                                        &control_block->ready_node);
        return status;
    }

    control_block->suspended_resume_state = HR_TASK_STATE_INVALID;
    hr_kernel_reload_time_slice(control_block);

    if ((g_current_task != NULL) && hr_task_is_valid(g_current_task))
    {
        current_control_block = hr_task_control_block(g_current_task);
#if (HR_CFG_PREEMPTION == 1)
        if ((current_control_block->state == HR_TASK_STATE_RUNNING) &&
            (control_block->effective_priority <
             current_control_block->effective_priority))
        {
            hr_kernel_mark_switch_reason(HR_SWITCH_REASON_PREEMPT);
            if (switch_required != NULL)
            {
                *switch_required = true;
            }
        }
#else
        (void)current_control_block;
#endif
    }

    return HR_OK;
}

static hr_status_t hr_kernel_make_task_ready(
    hr_task_control_block_t *control_block,
    hr_status_t result,
    bool *higher_priority_task_woken)
{
    hr_task_t *task;
    hr_task_control_block_t *current_control_block;
    hr_task_wait_cleanup_t cleanup;
    hr_status_t status;
    const bool suspended_wait =
        (control_block != NULL) &&
        (control_block->state == HR_TASK_STATE_SUSPENDED) &&
        (control_block->suspended_resume_state == HR_TASK_STATE_BLOCKED);

    if ((control_block == NULL) ||
        ((control_block->state != HR_TASK_STATE_BLOCKED) && !suspended_wait))
    {
        return HR_ERROR_INVALID_STATE;
    }

    task = (hr_task_t *)hr_list_node_owner(&control_block->ready_node.node);
    if (!hr_task_is_valid(task))
    {
        return HR_ERROR_INTERNAL;
    }

    cleanup = control_block->wait_cleanup;

    if (control_block->blocked_wait_list != NULL)
    {
        status = hr_wait_list_remove(control_block->blocked_wait_list,
                                     &control_block->wait_node);
        if (status != HR_OK)
        {
            return status;
        }
    }

    if (hr_list_node_is_linked(&control_block->timeout_node.node))
    {
        status = hr_timeout_list_remove(&g_timeout_list,
                                        &control_block->timeout_node);
        if (status != HR_OK)
        {
            return status;
        }
    }

    if (cleanup != NULL)
    {
        cleanup(control_block, result);
    }

    control_block->waiting_object = NULL;
    control_block->blocked_wait_list = NULL;
    control_block->wait_buffer = NULL;
    control_block->wait_cleanup = NULL;
    control_block->wait_result = result;
    control_block->wait_kind = HR_TASK_WAIT_NONE;
    hr_kernel_reload_time_slice(control_block);

    if (suspended_wait)
    {
        /* The wait completed, but administrative suspension wins. */
        control_block->suspended_resume_state = HR_TASK_STATE_READY;
        if (higher_priority_task_woken != NULL)
        {
            *higher_priority_task_woken = false;
        }
        return HR_OK;
    }

    status = hr_scheduler_add_ready(&g_scheduler, &control_block->ready_node);
    if (status != HR_OK)
    {
        return status;
    }

    status = hr_task_transition_state(task,
                                      HR_TASK_STATE_BLOCKED,
                                      HR_TASK_STATE_READY);
    if (status != HR_OK)
    {
        (void)hr_scheduler_remove_ready(&g_scheduler,
                                        &control_block->ready_node);
        return status;
    }

    if (higher_priority_task_woken != NULL)
    {
        *higher_priority_task_woken = false;
        if ((g_current_task != NULL) && hr_task_is_valid(g_current_task))
        {
            current_control_block = hr_task_control_block(g_current_task);
            *higher_priority_task_woken =
                (current_control_block->state == HR_TASK_STATE_RUNNING) &&
                (control_block->effective_priority <
                 current_control_block->effective_priority);
        }
    }

    return HR_OK;
}

hr_status_t hr_kernel_block_current_on_wait_list_ex(
    hr_wait_list_t *wait_list,
    void *object,
    hr_task_wait_kind_t wait_kind,
    void *buffer,
    hr_tick_t timeout,
    hr_task_wait_cleanup_t cleanup)
{
    hr_task_control_block_t *control_block;
    hr_status_t status;

    if ((g_kernel_state != HR_KERNEL_STATE_RUNNING) ||
        (g_current_task == NULL) || (g_current_task == &g_idle_task) ||
        (wait_list == NULL) || (object == NULL) ||
        ((wait_kind != HR_TASK_WAIT_QUEUE_SEND) &&
         (wait_kind != HR_TASK_WAIT_QUEUE_RECEIVE) &&
         (wait_kind != HR_TASK_WAIT_SEMAPHORE_TAKE) &&
         (wait_kind != HR_TASK_WAIT_MUTEX_LOCK)) ||
        (((wait_kind == HR_TASK_WAIT_QUEUE_SEND) ||
          (wait_kind == HR_TASK_WAIT_QUEUE_RECEIVE)) && (buffer == NULL)) ||
        (timeout == HR_NO_WAIT))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    control_block = hr_task_control_block(g_current_task);
    if ((control_block->state != HR_TASK_STATE_RUNNING) ||
        !hr_list_node_is_linked(&control_block->ready_node.node) ||
        hr_list_node_is_linked(&control_block->wait_node.node) ||
        hr_list_node_is_linked(&control_block->timeout_node.node))
    {
        return HR_ERROR_INVALID_STATE;
    }

    status = hr_scheduler_remove_ready(&g_scheduler,
                                       &control_block->ready_node);
    if (status != HR_OK)
    {
        return status;
    }

    status = hr_wait_list_insert(wait_list, &control_block->wait_node);
    if (status != HR_OK)
    {
        (void)hr_scheduler_add_ready(&g_scheduler,
                                     &control_block->ready_node);
        return status;
    }

    if (timeout != HR_WAIT_FOREVER)
    {
        status = hr_timeout_list_insert(&g_timeout_list,
                                        &control_block->timeout_node,
                                        timeout);
        if (status != HR_OK)
        {
            (void)hr_wait_list_remove(wait_list, &control_block->wait_node);
            (void)hr_scheduler_add_ready(&g_scheduler,
                                         &control_block->ready_node);
            return status;
        }
    }

    status = hr_task_transition_state(g_current_task,
                                      HR_TASK_STATE_RUNNING,
                                      HR_TASK_STATE_BLOCKED);
    if (status != HR_OK)
    {
        if (hr_list_node_is_linked(&control_block->timeout_node.node))
        {
            (void)hr_timeout_list_remove(&g_timeout_list,
                                         &control_block->timeout_node);
        }
        (void)hr_wait_list_remove(wait_list, &control_block->wait_node);
        (void)hr_scheduler_add_ready(&g_scheduler,
                                     &control_block->ready_node);
        return status;
    }

    control_block->wake_tick =
        (timeout == HR_WAIT_FOREVER) ? 0U : (g_kernel_tick + timeout);
    control_block->waiting_object = object;
    control_block->blocked_wait_list = wait_list;
    control_block->wait_buffer = buffer;
    control_block->wait_cleanup = cleanup;
    control_block->wait_result = HR_ERROR_INTERNAL;
    control_block->wait_kind = wait_kind;
    hr_kernel_reload_time_slice(control_block);
    hr_kernel_mark_switch_reason(HR_SWITCH_REASON_BLOCK);
    return HR_OK;
}

hr_status_t hr_kernel_block_current_on_wait_list(hr_wait_list_t *wait_list,
                                                 void *object,
                                                 hr_task_wait_kind_t wait_kind,
                                                 void *buffer,
                                                 hr_tick_t timeout)
{
    return hr_kernel_block_current_on_wait_list_ex(wait_list,
                                                   object,
                                                   wait_kind,
                                                   buffer,
                                                   timeout,
                                                   NULL);
}

hr_status_t hr_kernel_unblock_waiter(hr_wait_node_t *wait_node,
                                     hr_status_t result,
                                     bool *higher_priority_task_woken)
{
    hr_task_t *task;

    if (wait_node == NULL)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    task = (hr_task_t *)hr_list_node_owner(&wait_node->node);
    if (!hr_task_is_valid(task))
    {
        return HR_ERROR_INTERNAL;
    }

    return hr_kernel_make_task_ready(hr_task_control_block(task),
                                     result,
                                     higher_priority_task_woken);
}

void hr_kernel_select_next_from_pendsv(void)
{
    hr_ready_node_t *selected_ready_node;
    hr_ready_node_t *next_ready_node;
    hr_task_t *next_task;
    hr_task_control_block_t *current_control_block;
    hr_task_control_block_t *next_control_block;
    const uint32_t switch_reasons = g_switch_reasons;
    hr_status_t status;

    g_switch_reasons = HR_SWITCH_REASON_NONE;

    if ((g_kernel_state != HR_KERNEL_STATE_RUNNING) ||
        (g_current_task == NULL) ||
        (g_hr_current_task_control_block == NULL))
    {
        hr_kernel_panic();
        return;
    }

    current_control_block = hr_task_control_block(g_current_task);

    if (current_control_block->state == HR_TASK_STATE_RUNNING)
    {
        selected_ready_node = hr_scheduler_select_highest(&g_scheduler);
        if (selected_ready_node == NULL)
        {
            hr_kernel_panic();
            return;
        }

        if ((hr_list_node_owner(&selected_ready_node->node) == g_current_task) &&
            ((switch_reasons & (HR_SWITCH_REASON_YIELD |
                                HR_SWITCH_REASON_TIME_SLICE)) != 0U))
        {
            if ((switch_reasons & HR_SWITCH_REASON_YIELD) != 0U)
            {
                hr_kernel_reload_time_slice(current_control_block);
            }

            status = hr_scheduler_yield_current(&g_scheduler,
                                                &current_control_block->ready_node);
            if (status != HR_OK)
            {
                hr_kernel_panic();
                return;
            }
        }
    }
    else if (current_control_block->state == HR_TASK_STATE_READY)
    {
        /*
         * A one-tick timeout can expire after the task blocks but before the
         * pending PendSV runs. The task is READY again while its CPU context is
         * still active. The normal highest-ready selection resolves the race.
         */
    }
    else if ((current_control_block->state != HR_TASK_STATE_BLOCKED) &&
             (current_control_block->state != HR_TASK_STATE_SUSPENDED))
    {
        hr_kernel_panic();
        return;
    }

    next_ready_node = hr_scheduler_select_highest(&g_scheduler);
    if (next_ready_node == NULL)
    {
        hr_kernel_panic();
        return;
    }

    next_task = (hr_task_t *)hr_list_node_owner(&next_ready_node->node);
    if (!hr_task_is_valid(next_task))
    {
        hr_kernel_panic();
        return;
    }

    next_control_block = hr_task_control_block(next_task);
    if (next_task != g_current_task)
    {
        if (next_control_block->state != HR_TASK_STATE_READY)
        {
            hr_kernel_panic();
            return;
        }

        if ((current_control_block->state == HR_TASK_STATE_RUNNING) &&
            (hr_task_transition_state(g_current_task,
                                      HR_TASK_STATE_RUNNING,
                                      HR_TASK_STATE_READY) != HR_OK))
        {
            hr_kernel_panic();
            return;
        }

        if (hr_task_transition_state(next_task,
                                     HR_TASK_STATE_READY,
                                     HR_TASK_STATE_RUNNING) != HR_OK)
        {
            hr_kernel_panic();
            return;
        }
    }
    else if ((current_control_block->state == HR_TASK_STATE_READY) &&
             (hr_task_transition_state(g_current_task,
                                       HR_TASK_STATE_READY,
                                       HR_TASK_STATE_RUNNING) != HR_OK))
    {
        hr_kernel_panic();
        return;
    }

    g_current_task = next_task;
    g_hr_current_task_control_block = next_control_block;
}

void hr_kernel_tick_from_isr(void)
{
    hr_list_t expired_nodes;
    hr_list_node_t *node;
    hr_task_control_block_t *current_control_block;
    bool switch_required = false;

    if (g_kernel_state != HR_KERNEL_STATE_RUNNING)
    {
        return;
    }

    g_kernel_tick++;
    hr_list_init(&expired_nodes);

    if (hr_timeout_list_advance(&g_timeout_list,
                                g_kernel_tick,
                                &expired_nodes) != HR_OK)
    {
        hr_kernel_panic();
        return;
    }

    node = hr_list_pop_front(&expired_nodes);
    while (node != NULL)
    {
        hr_task_t *task = (hr_task_t *)hr_list_node_owner(node);
        hr_task_control_block_t *control_block;

        if (!hr_task_is_valid(task))
        {
            hr_kernel_panic();
            return;
        }

        control_block = hr_task_control_block(task);
        if ((control_block->state != HR_TASK_STATE_BLOCKED) &&
            !((control_block->state == HR_TASK_STATE_SUSPENDED) &&
              (control_block->suspended_resume_state ==
               HR_TASK_STATE_BLOCKED)))
        {
            hr_kernel_panic();
            return;
        }

        if (control_block->wait_kind == HR_TASK_WAIT_DELAY)
        {
            if (control_block->waiting_object != &g_timeout_list)
            {
                hr_kernel_panic();
                return;
            }

            if (hr_kernel_make_task_ready(control_block,
                                          HR_OK,
                                          NULL) != HR_OK)
            {
                hr_kernel_panic();
                return;
            }
        }
        else if ((control_block->wait_kind == HR_TASK_WAIT_QUEUE_SEND) ||
                 (control_block->wait_kind == HR_TASK_WAIT_QUEUE_RECEIVE) ||
                 (control_block->wait_kind == HR_TASK_WAIT_SEMAPHORE_TAKE) ||
                 (control_block->wait_kind == HR_TASK_WAIT_MUTEX_LOCK))
        {
            if ((control_block->waiting_object == NULL) ||
                (control_block->blocked_wait_list == NULL))
            {
                hr_kernel_panic();
                return;
            }

            if (hr_kernel_make_task_ready(control_block,
                                          HR_ERROR_TIMEOUT,
                                          NULL) != HR_OK)
            {
                hr_kernel_panic();
                return;
            }
        }
        else
        {
            hr_kernel_panic();
            return;
        }

        node = hr_list_pop_front(&expired_nodes);
    }

    if ((g_current_task == NULL) || !hr_task_is_valid(g_current_task))
    {
        hr_kernel_panic();
        return;
    }

    current_control_block = hr_task_control_block(g_current_task);
    if (current_control_block->state == HR_TASK_STATE_RUNNING)
    {
        current_control_block->runtime_counter++;

#if (HR_CFG_PREEMPTION == 1)
        if (hr_scheduler_should_preempt(&g_scheduler,
                                        &current_control_block->ready_node))
        {
            hr_kernel_mark_switch_reason(HR_SWITCH_REASON_PREEMPT);
            switch_required = true;
        }
#endif
#if (HR_CFG_TIME_SLICING == 1)
        if (!switch_required)
        {
            if (hr_scheduler_has_equal_priority_peer(&g_scheduler,
                                                     &current_control_block->ready_node))
            {
                if (current_control_block->time_slice_remaining > 0U)
                {
                    current_control_block->time_slice_remaining--;
                }

                if (current_control_block->time_slice_remaining == 0U)
                {
                    hr_kernel_reload_time_slice(current_control_block);
                    hr_kernel_mark_switch_reason(HR_SWITCH_REASON_TIME_SLICE);
                    switch_required = true;
                }
            }
            else
            {
                hr_kernel_reload_time_slice(current_control_block);
            }
        }
#endif
    }

    hr_port_yield_from_isr(switch_required);
}

hr_status_t hr_kernel_start(void)
{
    hr_status_t status;

    hr_port_configure_kernel_exceptions();

    status = hr_kernel_prepare_start();
    if (status != HR_OK)
    {
        return status;
    }

    hr_port_start_first_task();

    /* A successful SVC startup never returns here. */
    hr_kernel_panic();
    return HR_ERROR_INTERNAL;
}

bool hr_kernel_is_running(void)
{
    return g_kernel_state == HR_KERNEL_STATE_RUNNING;
}

hr_kernel_state_t hr_kernel_get_state(void)
{
    return g_kernel_state;
}

hr_tick_t hr_kernel_get_tick(void)
{
    return g_kernel_tick;
}

size_t hr_kernel_get_task_count(void)
{
    return g_task_count;
}

hr_status_t hr_kernel_set_task_effective_priority(
    hr_task_control_block_t *control_block,
    hr_priority_t priority)
{
    hr_status_t status;
    bool ready_linked;
    bool wait_linked;
    hr_wait_list_t *wait_list;

    if ((control_block == NULL) ||
        (priority >= (hr_priority_t)HR_CFG_IDLE_PRIORITY))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (control_block->effective_priority == priority)
    {
        return HR_OK;
    }

    ready_linked = hr_list_node_is_linked(&control_block->ready_node.node);
    wait_linked = hr_list_node_is_linked(&control_block->wait_node.node);
    wait_list = control_block->blocked_wait_list;

    if (ready_linked)
    {
        status = hr_scheduler_remove_ready(&g_scheduler,
                                           &control_block->ready_node);
        if (status != HR_OK)
        {
            return status;
        }
    }

    if (wait_linked)
    {
        if (wait_list == NULL)
        {
            if (ready_linked)
            {
                (void)hr_scheduler_add_ready(&g_scheduler,
                                             &control_block->ready_node);
            }
            return HR_ERROR_INTERNAL;
        }

        status = hr_wait_list_remove(wait_list, &control_block->wait_node);
        if (status != HR_OK)
        {
            if (ready_linked)
            {
                (void)hr_scheduler_add_ready(&g_scheduler,
                                             &control_block->ready_node);
            }
            return status;
        }
    }

    control_block->effective_priority = priority;
    control_block->ready_node.priority = priority;
    control_block->wait_node.priority = priority;

    if (ready_linked)
    {
        status = hr_scheduler_add_ready(&g_scheduler,
                                        &control_block->ready_node);
        if (status != HR_OK)
        {
            return status;
        }
    }

    if (wait_linked)
    {
        status = hr_wait_list_insert(wait_list, &control_block->wait_node);
        if (status != HR_OK)
        {
            return status;
        }
    }

    return HR_OK;
}

bool hr_kernel_current_should_preempt(void)
{
    hr_task_control_block_t *current;

    if ((g_kernel_state != HR_KERNEL_STATE_RUNNING) ||
        (g_current_task == NULL) || !hr_task_is_valid(g_current_task))
    {
        return false;
    }

    current = hr_task_control_block(g_current_task);
    return (current->state == HR_TASK_STATE_RUNNING) &&
           hr_scheduler_should_preempt(&g_scheduler, &current->ready_node);
}

hr_task_t *hr_kernel_current_task_internal(void)
{
    return g_current_task;
}
