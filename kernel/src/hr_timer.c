#include <stddef.h>

#include "hairtos_config.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_semaphore.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_timer.h"
#include "hr_list_internal.h"
#include "hr_port.h"
#include "hr_timeout_internal.h"
#include "hr_timer_internal.h"

#if (HR_CFG_ENABLE_SOFTWARE_TIMER == 1)

static bool g_timer_system_initialized;
static hr_timeout_list_t g_timer_timeouts;
static hr_list_t g_pending_timers;
static hr_semaphore_t g_timer_signal;
static hr_task_t g_timer_service_task;
static hr_stack_t g_timer_service_stack[HR_CFG_TIMER_TASK_STACK_WORDS];

static void hr_timer_zero_control_block(hr_timer_control_block_t *control_block)
{
    unsigned char *bytes = (unsigned char *)control_block;
    size_t index;

    for (index = 0U; index < sizeof(*control_block); index++)
    {
        bytes[index] = 0U;
    }
}

hr_timer_control_block_t *hr_timer_control_block(hr_timer_t *timer)
{
    return (timer == NULL) ? NULL :
           (hr_timer_control_block_t *)(void *)timer->storage;
}

const hr_timer_control_block_t *hr_timer_control_block_const(
    const hr_timer_t *timer)
{
    return (timer == NULL) ? NULL :
           (const hr_timer_control_block_t *)(const void *)timer->storage;
}

bool hr_timer_is_valid(const hr_timer_t *timer)
{
    const hr_timer_control_block_t *control_block =
        hr_timer_control_block_const(timer);

    return (control_block != NULL) &&
           (control_block->magic == HR_CFG_TIMER_MAGIC) &&
           (control_block->name != NULL) &&
           (control_block->callback != NULL) &&
           (control_block->period_ticks > 0U) &&
           (control_block->period_ticks != HR_WAIT_FOREVER);
}

bool hr_timer_validate_internal(const hr_timer_t *timer)
{
    const hr_timer_control_block_t *control_block =
        hr_timer_control_block_const(timer);

    if (!hr_timer_is_valid(timer))
    {
        return false;
    }

    if (control_block->active !=
        hr_list_node_is_linked(&control_block->timeout_node.node))
    {
        return false;
    }

    if ((control_block->pending_count == 0U) !=
        !hr_list_node_is_linked(&control_block->pending_node))
    {
        return false;
    }

    return true;
}

static void hr_timer_service_entry(void *argument)
{
    (void)argument;

    for (;;)
    {
        if (hr_semaphore_take(&g_timer_signal, HR_WAIT_FOREVER) != HR_OK)
        {
            for (;;)
            {
                hr_port_wait_for_interrupt();
            }
        }

        while (hr_timer_process_one_pending())
        {
            /* Drain every callback that became due before blocking again. */
        }
    }
}

void hr_timer_system_reset(void)
{
    g_timer_system_initialized = false;
}

hr_status_t hr_timer_system_ensure_initialized(void)
{
    hr_status_t status;

    if (g_timer_system_initialized)
    {
        return HR_OK;
    }

    if (hr_kernel_get_state() != HR_KERNEL_STATE_INITIALIZED)
    {
        return HR_ERROR_INVALID_STATE;
    }

    hr_timeout_list_init(&g_timer_timeouts, hr_kernel_get_tick());
    hr_list_init(&g_pending_timers);

    status = hr_semaphore_create_binary(&g_timer_signal, false);
    if (status != HR_OK)
    {
        return status;
    }

    status = hr_task_create_static(&g_timer_service_task,
                                   "timer-service",
                                   hr_timer_service_entry,
                                   NULL,
                                   g_timer_service_stack,
                                   HR_CFG_TIMER_TASK_STACK_WORDS,
                                   (hr_priority_t)HR_CFG_TIMER_TASK_PRIORITY);
    if (status != HR_OK)
    {
        return status;
    }

    status = hr_task_start(&g_timer_service_task);
    if (status != HR_OK)
    {
        return status;
    }

    g_timer_system_initialized = true;
    return HR_OK;
}

hr_status_t hr_timer_create_static(hr_timer_t *timer,
                                   const char *name,
                                   hr_tick_t period_ticks,
                                   bool auto_reload,
                                   hr_timer_callback_t callback,
                                   void *argument)
{
    hr_timer_control_block_t *control_block;
    hr_status_t status;

    if ((timer == NULL) || (name == NULL) || (callback == NULL) ||
        (period_ticks == 0U) || (period_ticks == HR_WAIT_FOREVER))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (hr_port_is_inside_isr())
    {
        return HR_ERROR_FROM_ISR;
    }

    if (hr_timer_is_valid(timer))
    {
        return HR_ERROR_INVALID_STATE;
    }

    status = hr_timer_system_ensure_initialized();
    if (status != HR_OK)
    {
        return status;
    }

    control_block = hr_timer_control_block(timer);
    hr_timer_zero_control_block(control_block);
    hr_timeout_node_init(&control_block->timeout_node, timer);
    hr_list_node_init(&control_block->pending_node, timer);
    control_block->name = name;
    control_block->callback = callback;
    control_block->argument = argument;
    control_block->period_ticks = period_ticks;
    control_block->auto_reload = auto_reload;
    control_block->active = false;
    control_block->callback_running = false;
    control_block->pending_count = 0U;
    control_block->magic = HR_CFG_TIMER_MAGIC;
    return HR_OK;
}

bool hr_timer_is_active(const hr_timer_t *timer)
{
    const hr_timer_control_block_t *control_block =
        hr_timer_control_block_const(timer);

    return hr_timer_is_valid(timer) && control_block->active;
}

const char *hr_timer_get_name(const hr_timer_t *timer)
{
    const hr_timer_control_block_t *control_block =
        hr_timer_control_block_const(timer);

    return hr_timer_is_valid(timer) ? control_block->name : NULL;
}

hr_tick_t hr_timer_get_period(const hr_timer_t *timer)
{
    const hr_timer_control_block_t *control_block =
        hr_timer_control_block_const(timer);

    return hr_timer_is_valid(timer) ? control_block->period_ticks : 0U;
}

uint32_t hr_timer_get_pending_count(const hr_timer_t *timer)
{
    const hr_timer_control_block_t *control_block =
        hr_timer_control_block_const(timer);

    return hr_timer_is_valid(timer) ? control_block->pending_count : 0U;
}

static void hr_timer_clear_pending_locked(hr_timer_control_block_t *control_block)
{
    if (hr_list_node_is_linked(&control_block->pending_node))
    {
        (void)hr_list_remove(&control_block->pending_node);
    }
    control_block->pending_count = 0U;
}

static hr_status_t hr_timer_arm_locked(hr_timer_control_block_t *control_block)
{
    hr_status_t status = hr_timeout_list_insert(&g_timer_timeouts,
                                                 &control_block->timeout_node,
                                                 control_block->period_ticks);
    if (status == HR_OK)
    {
        control_block->active = true;
    }
    return status;
}

hr_status_t hr_timer_start(hr_timer_t *timer)
{
    hr_timer_control_block_t *control_block;
    hr_irq_state_t irq_state;
    hr_status_t status;

    if (!hr_timer_is_valid(timer))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }
    if (hr_port_is_inside_isr())
    {
        return HR_ERROR_FROM_ISR;
    }

    control_block = hr_timer_control_block(timer);
    irq_state = hr_port_enter_critical();
    if (control_block->active)
    {
        status = HR_ERROR_INVALID_STATE;
    }
    else
    {
        hr_timer_clear_pending_locked(control_block);
        status = hr_timer_arm_locked(control_block);
    }
    hr_port_exit_critical(irq_state);
    return status;
}

hr_status_t hr_timer_stop(hr_timer_t *timer)
{
    hr_timer_control_block_t *control_block;
    hr_irq_state_t irq_state;
    hr_status_t status = HR_OK;

    if (!hr_timer_is_valid(timer))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }
    if (hr_port_is_inside_isr())
    {
        return HR_ERROR_FROM_ISR;
    }

    control_block = hr_timer_control_block(timer);
    irq_state = hr_port_enter_critical();
    if (control_block->active)
    {
        status = hr_timeout_list_remove(&g_timer_timeouts,
                                        &control_block->timeout_node);
        if (status == HR_OK)
        {
            control_block->active = false;
        }
    }
    hr_timer_clear_pending_locked(control_block);
    hr_port_exit_critical(irq_state);
    return status;
}

hr_status_t hr_timer_reset(hr_timer_t *timer)
{
    hr_timer_control_block_t *control_block;
    hr_irq_state_t irq_state;
    hr_status_t status = HR_OK;

    if (!hr_timer_is_valid(timer))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }
    if (hr_port_is_inside_isr())
    {
        return HR_ERROR_FROM_ISR;
    }

    control_block = hr_timer_control_block(timer);
    irq_state = hr_port_enter_critical();
    if (control_block->active)
    {
        status = hr_timeout_list_remove(&g_timer_timeouts,
                                        &control_block->timeout_node);
        if (status == HR_OK)
        {
            control_block->active = false;
        }
    }
    if (status == HR_OK)
    {
        hr_timer_clear_pending_locked(control_block);
        status = hr_timer_arm_locked(control_block);
    }
    hr_port_exit_critical(irq_state);
    return status;
}

hr_status_t hr_timer_change_period(hr_timer_t *timer,
                                   hr_tick_t new_period_ticks)
{
    hr_timer_control_block_t *control_block;
    hr_irq_state_t irq_state;
    hr_status_t status = HR_OK;

    if (!hr_timer_is_valid(timer) || (new_period_ticks == 0U) ||
        (new_period_ticks == HR_WAIT_FOREVER))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }
    if (hr_port_is_inside_isr())
    {
        return HR_ERROR_FROM_ISR;
    }

    control_block = hr_timer_control_block(timer);
    irq_state = hr_port_enter_critical();
    if (control_block->active)
    {
        status = hr_timeout_list_remove(&g_timer_timeouts,
                                        &control_block->timeout_node);
        if (status == HR_OK)
        {
            control_block->active = false;
        }
    }
    if (status == HR_OK)
    {
        control_block->period_ticks = new_period_ticks;
        hr_timer_clear_pending_locked(control_block);
        status = hr_timer_arm_locked(control_block);
    }
    hr_port_exit_critical(irq_state);
    return status;
}

void hr_timer_tick_from_isr(hr_tick_t now, bool *switch_required)
{
    hr_list_t expired;
    hr_list_node_t *node;
    bool signal_required = false;
    bool timer_task_woken = false;
    hr_status_t signal_status;

    if (switch_required != NULL)
    {
        *switch_required = false;
    }

    if (!g_timer_system_initialized)
    {
        return;
    }

    hr_list_init(&expired);
    if (hr_timeout_list_advance(&g_timer_timeouts, now, &expired) != HR_OK)
    {
        return;
    }

    node = hr_list_pop_front(&expired);
    while (node != NULL)
    {
        hr_timer_t *timer = (hr_timer_t *)hr_list_node_owner(node);
        hr_timer_control_block_t *control_block = hr_timer_control_block(timer);

        if (!hr_timer_is_valid(timer) || !control_block->active)
        {
            return;
        }

        control_block->active = false;
        if (control_block->pending_count < UINT32_MAX)
        {
            control_block->pending_count++;
        }
        if (!hr_list_node_is_linked(&control_block->pending_node))
        {
            (void)hr_list_push_back(&g_pending_timers,
                                    &control_block->pending_node);
        }
        signal_required = true;

        if (control_block->auto_reload)
        {
            if (hr_timer_arm_locked(control_block) != HR_OK)
            {
                return;
            }
        }

        node = hr_list_pop_front(&expired);
    }

    if (signal_required)
    {
        signal_status = hr_semaphore_give_from_isr(&g_timer_signal,
                                                   &timer_task_woken);
        if ((signal_status != HR_OK) &&
            (signal_status != HR_ERROR_SEMAPHORE_FULL))
        {
            return;
        }
    }

    if (switch_required != NULL)
    {
        *switch_required = timer_task_woken;
    }
}

bool hr_timer_process_one_pending(void)
{
    hr_list_node_t *node;
    hr_timer_t *timer;
    hr_timer_control_block_t *control_block;
    hr_timer_callback_t callback;
    void *argument;
    hr_irq_state_t irq_state;

    if (!g_timer_system_initialized)
    {
        return false;
    }

    irq_state = hr_port_enter_critical();
    node = hr_list_pop_front(&g_pending_timers);
    if (node == NULL)
    {
        hr_port_exit_critical(irq_state);
        return false;
    }

    timer = (hr_timer_t *)hr_list_node_owner(node);
    control_block = hr_timer_control_block(timer);
    if (!hr_timer_is_valid(timer) || (control_block->pending_count == 0U))
    {
        hr_port_exit_critical(irq_state);
        return false;
    }

    control_block->pending_count--;
    if (control_block->pending_count > 0U)
    {
        (void)hr_list_push_back(&g_pending_timers,
                                &control_block->pending_node);
    }
    callback = control_block->callback;
    argument = control_block->argument;
    control_block->callback_running = true;
    hr_port_exit_critical(irq_state);

    callback(argument);

    irq_state = hr_port_enter_critical();
    control_block->callback_running = false;
    hr_port_exit_critical(irq_state);
    return true;
}

#else

hr_timer_control_block_t *hr_timer_control_block(hr_timer_t *timer)
{
    (void)timer;
    return NULL;
}

const hr_timer_control_block_t *hr_timer_control_block_const(
    const hr_timer_t *timer)
{
    (void)timer;
    return NULL;
}

void hr_timer_system_reset(void) {}
hr_status_t hr_timer_system_ensure_initialized(void) { return HR_ERROR_NOT_SUPPORTED; }
void hr_timer_tick_from_isr(hr_tick_t now, bool *switch_required)
{
    (void)now;
    if (switch_required != NULL) { *switch_required = false; }
}
bool hr_timer_process_one_pending(void) { return false; }
bool hr_timer_validate_internal(const hr_timer_t *timer) { (void)timer; return false; }
hr_status_t hr_timer_create_static(hr_timer_t *timer, const char *name,
                                   hr_tick_t period_ticks, bool auto_reload,
                                   hr_timer_callback_t callback, void *argument)
{
    (void)timer; (void)name; (void)period_ticks; (void)auto_reload;
    (void)callback; (void)argument; return HR_ERROR_NOT_SUPPORTED;
}
bool hr_timer_is_valid(const hr_timer_t *timer) { (void)timer; return false; }
bool hr_timer_is_active(const hr_timer_t *timer) { (void)timer; return false; }
const char *hr_timer_get_name(const hr_timer_t *timer) { (void)timer; return NULL; }
hr_tick_t hr_timer_get_period(const hr_timer_t *timer) { (void)timer; return 0U; }
uint32_t hr_timer_get_pending_count(const hr_timer_t *timer) { (void)timer; return 0U; }
hr_status_t hr_timer_start(hr_timer_t *timer) { (void)timer; return HR_ERROR_NOT_SUPPORTED; }
hr_status_t hr_timer_stop(hr_timer_t *timer) { (void)timer; return HR_ERROR_NOT_SUPPORTED; }
hr_status_t hr_timer_reset(hr_timer_t *timer) { (void)timer; return HR_ERROR_NOT_SUPPORTED; }
hr_status_t hr_timer_change_period(hr_timer_t *timer, hr_tick_t period)
{ (void)timer; (void)period; return HR_ERROR_NOT_SUPPORTED; }

#endif
