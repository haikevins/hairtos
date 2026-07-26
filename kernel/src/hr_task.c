#include <stddef.h>
#include <stdint.h>

#include "hairtos_config.h"
#include "hairtos/hr_task.h"
#include "hr_port.h"
#include "hr_kernel_internal.h"
#include "hr_task_internal.h"

#define HR_STACK_FILL_WORD 0xA5A5A5A5UL

static void hr_task_zero_control_block(hr_task_control_block_t *control_block)
{
    unsigned char *bytes = (unsigned char *)control_block;
    size_t index;

    for (index = 0U; index < sizeof(*control_block); index++)
    {
        bytes[index] = 0U;
    }
}

static void hr_task_fill_stack(hr_stack_t *stack, size_t stack_words)
{
    size_t index;

    for (index = 0U; index < stack_words; index++)
    {
        stack[index] = HR_STACK_FILL_WORD;
    }

    stack[0] = HR_CFG_STACK_GUARD_VALUE;
}

hr_task_control_block_t *hr_task_control_block(hr_task_t *task)
{
    return (task == NULL) ? NULL : (hr_task_control_block_t *)(void *)task->storage;
}

const hr_task_control_block_t *hr_task_control_block_const(const hr_task_t *task)
{
    return (task == NULL) ? NULL : (const hr_task_control_block_t *)(const void *)task->storage;
}

hr_status_t hr_task_transition_state(hr_task_t *task,
                                          hr_task_state_t expected,
                                          hr_task_state_t next)
{
    hr_task_control_block_t *control_block;

    if (!hr_task_is_valid(task))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    control_block = hr_task_control_block(task);
    if (control_block->state != expected)
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block->state = next;
    return HR_OK;
}

void hr_task_exit_error(void)
{
    for (;;)
    {
        /* A task entry function must never return. */
    }
}

hr_status_t hr_task_create_static(hr_task_t *task,
                                  const char *name,
                                  hr_task_entry_t entry,
                                  void *argument,
                                  hr_stack_t *stack,
                                  size_t stack_words,
                                  hr_priority_t priority)
{
    hr_task_control_block_t *control_block;
    hr_stack_t *initial_stack_pointer;

    if ((task == NULL) || (name == NULL) || (entry == NULL) || (stack == NULL))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (hr_task_is_valid(task))
    {
        return HR_ERROR_INVALID_STATE;
    }

    if ((stack_words < HR_CFG_MIN_TASK_STACK_WORDS) ||
        (priority >= HR_CFG_PRIORITY_COUNT))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    control_block = hr_task_control_block(task);
    hr_task_zero_control_block(control_block);
    hr_task_fill_stack(stack, stack_words);

    initial_stack_pointer = hr_port_initialize_stack(stack,
                                                     stack_words,
                                                     entry,
                                                     argument,
                                                     hr_task_exit_error);
    if (initial_stack_pointer == NULL)
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block->stack_pointer = initial_stack_pointer;
    control_block->stack_low = stack;
    control_block->stack_high = &stack[stack_words - 1U];
    control_block->name = name;
    control_block->entry = entry;
    control_block->argument = argument;
    control_block->state = HR_TASK_STATE_CREATED;
    control_block->suspended_resume_state = HR_TASK_STATE_INVALID;
    control_block->base_priority = priority;
    control_block->effective_priority = priority;
    control_block->wake_tick = 0U;
    control_block->time_slice_remaining = HR_CFG_TIME_SLICE_TICKS;
    control_block->waiting_object = NULL;
    control_block->blocked_wait_list = NULL;
    control_block->wait_buffer = NULL;
    control_block->wait_cleanup = NULL;
    control_block->wait_result = HR_OK;
    control_block->wait_kind = HR_TASK_WAIT_NONE;
    hr_list_init(&control_block->owned_mutexes);
    control_block->owned_mutex_count = 0U;
    control_block->stack_words = stack_words;
    control_block->critical_nesting = 0U;
    control_block->runtime_counter = 0U;

    hr_ready_node_init(&control_block->ready_node, task, priority);
    hr_wait_node_init(&control_block->wait_node, task, priority);
    hr_timeout_node_init(&control_block->timeout_node, task);
    hr_list_node_init(&control_block->all_task_node, task);

    control_block->magic = HR_CFG_TASK_MAGIC;
    return HR_OK;
}


hr_status_t hr_task_start(hr_task_t *task)
{
    if (!hr_task_is_valid(task))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    return hr_kernel_register_task(task);
}

bool hr_task_is_valid(const hr_task_t *task)
{
    const hr_task_control_block_t *control_block = hr_task_control_block_const(task);

    return (control_block != NULL) && (control_block->magic == HR_CFG_TASK_MAGIC);
}

const char *hr_task_get_name(const hr_task_t *task)
{
    const hr_task_control_block_t *control_block = hr_task_control_block_const(task);

    return hr_task_is_valid(task) ? control_block->name : NULL;
}

hr_task_state_t hr_task_get_state(const hr_task_t *task)
{
    const hr_task_control_block_t *control_block = hr_task_control_block_const(task);

    return hr_task_is_valid(task) ? control_block->state : HR_TASK_STATE_INVALID;
}

hr_priority_t hr_task_get_base_priority(const hr_task_t *task)
{
    const hr_task_control_block_t *control_block = hr_task_control_block_const(task);

    return hr_task_is_valid(task) ? control_block->base_priority : 0U;
}

hr_priority_t hr_task_get_effective_priority(const hr_task_t *task)
{
    const hr_task_control_block_t *control_block = hr_task_control_block_const(task);

    return hr_task_is_valid(task) ? control_block->effective_priority : 0U;
}

size_t hr_task_get_stack_words(const hr_task_t *task)
{
    const hr_task_control_block_t *control_block = hr_task_control_block_const(task);

    return hr_task_is_valid(task) ? control_block->stack_words : 0U;
}

size_t hr_task_get_stack_high_watermark(const hr_task_t *task)
{
    const hr_task_control_block_t *control_block = hr_task_control_block_const(task);
    size_t free_words = 0U;
    size_t index;

    if (!hr_task_is_valid(task) || (control_block->stack_low == NULL))
    {
        return 0U;
    }

    for (index = 1U; index < control_block->stack_words; index++)
    {
        if (control_block->stack_low[index] != HR_STACK_FILL_WORD)
        {
            break;
        }
        free_words++;
    }

    return free_words;
}

bool hr_task_stack_guard_is_valid(const hr_task_t *task)
{
    const hr_task_control_block_t *control_block = hr_task_control_block_const(task);

    return hr_task_is_valid(task) &&
           (control_block->stack_low != NULL) &&
           (control_block->stack_low[0] == HR_CFG_STACK_GUARD_VALUE);
}



void hr_task_yield(void)
{
    if (hr_kernel_is_running() && !hr_port_is_inside_isr())
    {
        hr_kernel_request_yield();
    }
}

hr_status_t hr_task_delay(hr_tick_t ticks)
{
    hr_irq_state_t irq_state;
    hr_status_t status;

    if (!hr_kernel_is_running() || hr_port_is_inside_isr())
    {
        return HR_ERROR_INVALID_STATE;
    }

    if (ticks == 0U)
    {
        hr_task_yield();
        return HR_OK;
    }

    if (ticks == HR_WAIT_FOREVER)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    irq_state = hr_port_enter_critical();
    status = hr_kernel_delay_current(ticks);
    hr_port_exit_critical(irq_state);

    if (status == HR_OK)
    {
        hr_port_request_context_switch();
    }

    return status;
}

hr_status_t hr_task_delay_until(hr_tick_t *last_wake_tick, hr_tick_t period)
{
    hr_irq_state_t irq_state;
    hr_tick_t now;
    hr_tick_t next_wake;
    hr_tick_t remaining_ticks;
    bool should_block;
    hr_status_t status = HR_OK;

    if (last_wake_tick == NULL)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (!hr_kernel_is_running() || hr_port_is_inside_isr())
    {
        return HR_ERROR_INVALID_STATE;
    }

    if ((period == 0U) || (period > (hr_tick_t)INT32_MAX))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    irq_state = hr_port_enter_critical();
    now = hr_kernel_get_tick();
    next_wake = *last_wake_tick + period;
    remaining_ticks = next_wake - now;
    should_block = (remaining_ticks != 0U) &&
                   (remaining_ticks <= (hr_tick_t)INT32_MAX);

    if (should_block)
    {
        status = hr_kernel_delay_current(remaining_ticks);
    }

    if (status == HR_OK)
    {
        *last_wake_tick = next_wake;
    }

    hr_port_exit_critical(irq_state);

    if ((status == HR_OK) && should_block)
    {
        hr_port_request_context_switch();
    }

    return status;
}


hr_status_t hr_task_suspend(hr_task_t *task)
{
    hr_irq_state_t irq_state;
    hr_status_t status;
    bool switch_required = false;

    if (!hr_task_is_valid(task))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (!hr_kernel_is_running() || hr_port_is_inside_isr())
    {
        return hr_port_is_inside_isr() ? HR_ERROR_FROM_ISR :
                                         HR_ERROR_INVALID_STATE;
    }

    irq_state = hr_port_enter_critical();
    status = hr_kernel_suspend_task(task, &switch_required);
    hr_port_exit_critical(irq_state);

    if ((status == HR_OK) && switch_required)
    {
        hr_port_request_context_switch();
    }

    return status;
}

hr_status_t hr_task_resume(hr_task_t *task)
{
    hr_irq_state_t irq_state;
    hr_status_t status;
    bool switch_required = false;

    if (!hr_task_is_valid(task))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (!hr_kernel_is_running() || hr_port_is_inside_isr())
    {
        return hr_port_is_inside_isr() ? HR_ERROR_FROM_ISR :
                                         HR_ERROR_INVALID_STATE;
    }

    irq_state = hr_port_enter_critical();
    status = hr_kernel_resume_task(task, &switch_required);
    hr_port_exit_critical(irq_state);

    if ((status == HR_OK) && switch_required)
    {
        hr_port_request_context_switch();
    }

    return status;
}

hr_task_t *hr_task_current(void)
{
    return hr_kernel_current_task_internal();
}
