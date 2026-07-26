#include <stddef.h>

#include "hairtos_config.h"
#include "hairtos/hr_semaphore.h"
#include "hr_kernel_internal.h"
#include "hr_port.h"
#include "hr_semaphore_internal.h"
#include "hr_task_internal.h"

static void hr_semaphore_zero_control_block(
    hr_semaphore_control_block_t *control_block)
{
    unsigned char *bytes = (unsigned char *)control_block;
    size_t index;

    for (index = 0U; index < sizeof(*control_block); index++)
    {
        bytes[index] = 0U;
    }
}

hr_semaphore_control_block_t *hr_semaphore_control_block(
    hr_semaphore_t *semaphore)
{
    return (semaphore == NULL) ? NULL :
           (hr_semaphore_control_block_t *)(void *)semaphore->storage;
}

const hr_semaphore_control_block_t *hr_semaphore_control_block_const(
    const hr_semaphore_t *semaphore)
{
    return (semaphore == NULL) ? NULL :
           (const hr_semaphore_control_block_t *)(const void *)semaphore->storage;
}

bool hr_semaphore_is_valid(const hr_semaphore_t *semaphore)
{
    const hr_semaphore_control_block_t *control_block =
        hr_semaphore_control_block_const(semaphore);

    return (control_block != NULL) &&
           (control_block->magic == HR_CFG_SEMAPHORE_MAGIC) &&
           (control_block->max_count > 0U) &&
           (control_block->count <= control_block->max_count);
}

bool hr_semaphore_validate_internal(const hr_semaphore_t *semaphore)
{
    const hr_semaphore_control_block_t *control_block =
        hr_semaphore_control_block_const(semaphore);

    return hr_semaphore_is_valid(semaphore) &&
           hr_wait_list_validate(&control_block->waiters);
}

hr_status_t hr_semaphore_create_counting(hr_semaphore_t *semaphore,
                                         uint32_t initial_count,
                                         uint32_t max_count)
{
    hr_semaphore_control_block_t *control_block;

    if ((semaphore == NULL) || (max_count == 0U) ||
        (initial_count > max_count))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (hr_semaphore_is_valid(semaphore))
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = hr_semaphore_control_block(semaphore);
    hr_semaphore_zero_control_block(control_block);
    control_block->count = initial_count;
    control_block->max_count = max_count;
    hr_wait_list_init(&control_block->waiters);
    control_block->magic = HR_CFG_SEMAPHORE_MAGIC;
    return HR_OK;
}

hr_status_t hr_semaphore_create_binary(hr_semaphore_t *semaphore,
                                       bool initially_available)
{
    return hr_semaphore_create_counting(semaphore,
                                        initially_available ? 1U : 0U,
                                        1U);
}

uint32_t hr_semaphore_get_count(const hr_semaphore_t *semaphore)
{
    const hr_semaphore_control_block_t *control_block =
        hr_semaphore_control_block_const(semaphore);

    return hr_semaphore_is_valid(semaphore) ? control_block->count : 0U;
}

uint32_t hr_semaphore_get_max_count(const hr_semaphore_t *semaphore)
{
    const hr_semaphore_control_block_t *control_block =
        hr_semaphore_control_block_const(semaphore);

    return hr_semaphore_is_valid(semaphore) ? control_block->max_count : 0U;
}

size_t hr_semaphore_get_waiting_tasks(const hr_semaphore_t *semaphore)
{
    const hr_semaphore_control_block_t *control_block =
        hr_semaphore_control_block_const(semaphore);

    return hr_semaphore_is_valid(semaphore) ?
           hr_wait_list_size(&control_block->waiters) : 0U;
}

static hr_status_t hr_semaphore_give_locked(
    hr_semaphore_control_block_t *control_block,
    bool *higher_priority_task_woken)
{
    hr_wait_node_t *waiter;

    waiter = hr_wait_list_peek(&control_block->waiters);
    if (waiter != NULL)
    {
        return hr_kernel_unblock_waiter(waiter,
                                        HR_OK,
                                        higher_priority_task_woken);
    }

    if (control_block->count >= control_block->max_count)
    {
        return HR_ERROR_SEMAPHORE_FULL;
    }

    control_block->count++;
    if (higher_priority_task_woken != NULL)
    {
        *higher_priority_task_woken = false;
    }
    return HR_OK;
}

hr_status_t hr_semaphore_take(hr_semaphore_t *semaphore, hr_tick_t timeout)
{
    hr_semaphore_control_block_t *control_block;
    hr_task_control_block_t *blocked_task = NULL;
    hr_irq_state_t irq_state;
    hr_status_t status;
    bool blocked = false;

    if (!hr_semaphore_is_valid(semaphore))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (hr_port_is_inside_isr())
    {
        return HR_ERROR_FROM_ISR;
    }

    if ((timeout != HR_NO_WAIT) && !hr_kernel_is_running())
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = hr_semaphore_control_block(semaphore);
    irq_state = hr_port_enter_critical();

    if (control_block->count > 0U)
    {
        control_block->count--;
        status = HR_OK;
    }
    else if (timeout == HR_NO_WAIT)
    {
        status = HR_ERROR_SEMAPHORE_EMPTY;
    }
    else
    {
        blocked_task = hr_task_control_block(hr_task_current());
        if (blocked_task == NULL)
        {
            status = HR_ERROR_INVALID_STATE;
        }
        else
        {
            status = hr_kernel_block_current_on_wait_list_ex(
                &control_block->waiters,
                control_block,
                HR_TASK_WAIT_SEMAPHORE_TAKE,
                NULL,
                timeout,
                NULL);
            blocked = (status == HR_OK);
        }
    }

    hr_port_exit_critical(irq_state);

    if (blocked)
    {
        hr_port_request_context_switch();
        return blocked_task->wait_result;
    }

    return status;
}

hr_status_t hr_semaphore_give(hr_semaphore_t *semaphore)
{
    hr_semaphore_control_block_t *control_block;
    hr_irq_state_t irq_state;
    hr_status_t status;
    bool higher_priority_task_woken = false;

    if (!hr_semaphore_is_valid(semaphore))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (hr_port_is_inside_isr())
    {
        return HR_ERROR_FROM_ISR;
    }

    control_block = hr_semaphore_control_block(semaphore);
    irq_state = hr_port_enter_critical();
    status = hr_semaphore_give_locked(control_block,
                                      &higher_priority_task_woken);
    hr_port_exit_critical(irq_state);

    if (higher_priority_task_woken)
    {
        hr_port_request_context_switch();
    }

    return status;
}

hr_status_t hr_semaphore_give_from_isr(hr_semaphore_t *semaphore,
                                       bool *higher_priority_task_woken)
{
    hr_semaphore_control_block_t *control_block;
    hr_irq_state_t irq_state;
    hr_status_t status;
    bool switch_required = false;

    if (!hr_semaphore_is_valid(semaphore))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (!hr_port_is_inside_isr())
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = hr_semaphore_control_block(semaphore);
    irq_state = hr_port_enter_critical();
    status = hr_semaphore_give_locked(control_block, &switch_required);
    hr_port_exit_critical(irq_state);

    if (higher_priority_task_woken != NULL)
    {
        *higher_priority_task_woken = switch_required;
    }

    return status;
}
