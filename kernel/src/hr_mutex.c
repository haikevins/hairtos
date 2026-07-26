#include <stddef.h>
#include <stdint.h>

#include "hairtos_config.h"
#include "hairtos/hr_mutex.h"
#include "hr_kernel_internal.h"
#include "hr_mutex_internal.h"
#include "hr_port.h"
#include "hr_task_internal.h"

static void hr_mutex_zero_control_block(hr_mutex_control_block_t *control_block)
{
    unsigned char *bytes = (unsigned char *)control_block;
    size_t index;

    for (index = 0U; index < sizeof(*control_block); index++)
    {
        bytes[index] = 0U;
    }
}

hr_mutex_control_block_t *hr_mutex_control_block(hr_mutex_t *mutex)
{
    return (mutex == NULL) ? NULL :
           (hr_mutex_control_block_t *)(void *)mutex->storage;
}

const hr_mutex_control_block_t *hr_mutex_control_block_const(
    const hr_mutex_t *mutex)
{
    return (mutex == NULL) ? NULL :
           (const hr_mutex_control_block_t *)(const void *)mutex->storage;
}

bool hr_mutex_is_valid(const hr_mutex_t *mutex)
{
    const hr_mutex_control_block_t *control_block =
        hr_mutex_control_block_const(mutex);

    return (control_block != NULL) &&
           (control_block->magic == HR_CFG_MUTEX_MAGIC) &&
           (((control_block->owner == NULL) &&
             (control_block->recursion_count == 0U) &&
             !hr_list_node_is_linked(&control_block->owner_node)) ||
            ((control_block->owner != NULL) &&
             hr_task_is_valid(control_block->owner) &&
             (control_block->recursion_count > 0U) &&
             hr_list_node_is_linked(&control_block->owner_node)));
}

bool hr_mutex_validate_internal(const hr_mutex_t *mutex)
{
    const hr_mutex_control_block_t *control_block =
        hr_mutex_control_block_const(mutex);

    if (!hr_mutex_is_valid(mutex) ||
        !hr_wait_list_validate(&control_block->waiters))
    {
        return false;
    }

    if (control_block->owner != NULL)
    {
        const hr_task_control_block_t *owner =
            hr_task_control_block_const(control_block->owner);
        return control_block->owner_node.list == &owner->owned_mutexes;
    }

    return true;
}

static hr_status_t hr_mutex_create_internal(hr_mutex_t *mutex, bool recursive)
{
    hr_mutex_control_block_t *control_block;

    if (mutex == NULL)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (hr_mutex_is_valid(mutex))
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = hr_mutex_control_block(mutex);
    hr_mutex_zero_control_block(control_block);
    control_block->recursive = recursive;
    hr_wait_list_init(&control_block->waiters);
    hr_list_node_init(&control_block->owner_node, mutex);
    control_block->magic = HR_CFG_MUTEX_MAGIC;
    return HR_OK;
}

hr_status_t hr_mutex_create(hr_mutex_t *mutex)
{
    return hr_mutex_create_internal(mutex, false);
}

hr_status_t hr_mutex_create_recursive(hr_mutex_t *mutex)
{
    return hr_mutex_create_internal(mutex, true);
}

bool hr_mutex_is_recursive(const hr_mutex_t *mutex)
{
    const hr_mutex_control_block_t *control_block =
        hr_mutex_control_block_const(mutex);

    return hr_mutex_is_valid(mutex) && control_block->recursive;
}

hr_task_t *hr_mutex_get_owner(const hr_mutex_t *mutex)
{
    const hr_mutex_control_block_t *control_block =
        hr_mutex_control_block_const(mutex);

    return hr_mutex_is_valid(mutex) ? control_block->owner : NULL;
}

uint32_t hr_mutex_get_recursion_count(const hr_mutex_t *mutex)
{
    const hr_mutex_control_block_t *control_block =
        hr_mutex_control_block_const(mutex);

    return hr_mutex_is_valid(mutex) ? control_block->recursion_count : 0U;
}

size_t hr_mutex_get_waiting_tasks(const hr_mutex_t *mutex)
{
    const hr_mutex_control_block_t *control_block =
        hr_mutex_control_block_const(mutex);

    return hr_mutex_is_valid(mutex) ?
           hr_wait_list_size(&control_block->waiters) : 0U;
}

static hr_status_t hr_mutex_attach_owner(hr_mutex_control_block_t *mutex,
                                         hr_task_t *owner_task)
{
    hr_task_control_block_t *owner;
    hr_status_t status;

    if ((mutex == NULL) || !hr_task_is_valid(owner_task) ||
        (mutex->owner != NULL) ||
        hr_list_node_is_linked(&mutex->owner_node))
    {
        return HR_ERROR_INVALID_STATE;
    }

    owner = hr_task_control_block(owner_task);
    status = hr_list_push_back(&owner->owned_mutexes, &mutex->owner_node);
    if (status != HR_OK)
    {
        return status;
    }

    mutex->owner = owner_task;
    mutex->recursion_count = 1U;
    owner->owned_mutex_count++;
    return HR_OK;
}

static hr_status_t hr_mutex_detach_owner(hr_mutex_control_block_t *mutex,
                                         hr_task_control_block_t **old_owner)
{
    hr_task_control_block_t *owner;
    hr_status_t status;

    if ((mutex == NULL) || (mutex->owner == NULL) ||
        !hr_task_is_valid(mutex->owner) ||
        !hr_list_node_is_linked(&mutex->owner_node))
    {
        return HR_ERROR_INVALID_STATE;
    }

    owner = hr_task_control_block(mutex->owner);
    if ((owner->owned_mutex_count == 0U) ||
        (mutex->owner_node.list != &owner->owned_mutexes))
    {
        return HR_ERROR_INTERNAL;
    }

    status = hr_list_remove(&mutex->owner_node);
    if (status != HR_OK)
    {
        return status;
    }

    owner->owned_mutex_count--;
    mutex->owner = NULL;
    mutex->recursion_count = 0U;
    if (old_owner != NULL)
    {
        *old_owner = owner;
    }
    return HR_OK;
}

static hr_priority_t hr_mutex_required_priority(
    const hr_task_control_block_t *task)
{
    hr_priority_t priority;
    hr_list_node_t *node;

    priority = task->base_priority;
    node = hr_list_front(&task->owned_mutexes);
    while (node != NULL)
    {
        const hr_mutex_t *mutex_object =
            (const hr_mutex_t *)hr_list_node_owner(node);
        const hr_mutex_control_block_t *mutex =
            hr_mutex_control_block_const(mutex_object);
        hr_wait_node_t *waiter;

        if ((mutex == NULL) || (mutex->magic != HR_CFG_MUTEX_MAGIC))
        {
            return task->base_priority;
        }

        waiter = hr_wait_list_peek(&mutex->waiters);
        if ((waiter != NULL) && (waiter->priority < priority))
        {
            priority = waiter->priority;
        }

        node = hr_list_next(&task->owned_mutexes, node);
    }

    return priority;
}

static hr_status_t hr_mutex_recompute_task_priority(
    hr_task_control_block_t *task,
    size_t depth)
{
    hr_priority_t required;
    hr_status_t status;

    if ((task == NULL) || (depth > (size_t)HR_CFG_MAX_TASKS))
    {
        return HR_ERROR_INTERNAL;
    }

    required = hr_mutex_required_priority(task);
    status = hr_kernel_set_task_effective_priority(task, required);
    if (status != HR_OK)
    {
        return status;
    }

    if ((task->state == HR_TASK_STATE_BLOCKED) &&
        (task->wait_kind == HR_TASK_WAIT_MUTEX_LOCK) &&
        (task->waiting_object != NULL))
    {
        hr_mutex_control_block_t *waiting_mutex =
            (hr_mutex_control_block_t *)task->waiting_object;

        if ((waiting_mutex->magic == HR_CFG_MUTEX_MAGIC) &&
            (waiting_mutex->owner != NULL) &&
            hr_task_is_valid(waiting_mutex->owner))
        {
            hr_task_control_block_t *owner =
                hr_task_control_block(waiting_mutex->owner);
            if (owner != task)
            {
                return hr_mutex_recompute_task_priority(owner, depth + 1U);
            }
        }
    }

    return HR_OK;
}

hr_status_t hr_mutex_recompute_owner_priority(hr_mutex_control_block_t *mutex)
{
    if ((mutex == NULL) || (mutex->magic != HR_CFG_MUTEX_MAGIC))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (mutex->owner == NULL)
    {
        return HR_OK;
    }

    if (!hr_task_is_valid(mutex->owner))
    {
        return HR_ERROR_INTERNAL;
    }

    return hr_mutex_recompute_task_priority(
        hr_task_control_block(mutex->owner),
        0U);
}

void hr_mutex_wait_cleanup(struct hr_task_control_block *task,
                           hr_status_t result)
{
    hr_mutex_control_block_t *mutex;

    if ((task == NULL) || (result != HR_ERROR_TIMEOUT) ||
        (task->wait_kind != HR_TASK_WAIT_MUTEX_LOCK) ||
        (task->waiting_object == NULL))
    {
        return;
    }

    mutex = (hr_mutex_control_block_t *)task->waiting_object;
    (void)hr_mutex_recompute_owner_priority(mutex);
}

hr_status_t hr_mutex_lock(hr_mutex_t *mutex_object, hr_tick_t timeout)
{
    hr_mutex_control_block_t *mutex;
    hr_task_t *current_task;
    hr_task_control_block_t *current;
    hr_irq_state_t irq_state;
    hr_status_t status;
    bool blocked = false;

    if (!hr_mutex_is_valid(mutex_object))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (hr_port_is_inside_isr())
    {
        return HR_ERROR_FROM_ISR;
    }

    if (!hr_kernel_is_running())
    {
        return HR_ERROR_INVALID_STATE;
    }

    current_task = hr_task_current();
    if (!hr_task_is_valid(current_task))
    {
        return HR_ERROR_INVALID_STATE;
    }

    current = hr_task_control_block(current_task);
    mutex = hr_mutex_control_block(mutex_object);
    irq_state = hr_port_enter_critical();

    if (mutex->owner == NULL)
    {
        status = hr_mutex_attach_owner(mutex, current_task);
    }
    else if (mutex->owner == current_task)
    {
        if (!mutex->recursive)
        {
            status = HR_ERROR_MUTEX_BUSY;
        }
        else if (mutex->recursion_count == UINT32_MAX)
        {
            status = HR_ERROR_OVERFLOW;
        }
        else
        {
            mutex->recursion_count++;
            status = HR_OK;
        }
    }
    else if (timeout == HR_NO_WAIT)
    {
        status = HR_ERROR_MUTEX_BUSY;
    }
    else
    {
        status = hr_kernel_block_current_on_wait_list_ex(
            &mutex->waiters,
            mutex,
            HR_TASK_WAIT_MUTEX_LOCK,
            NULL,
            timeout,
            hr_mutex_wait_cleanup);
        if (status == HR_OK)
        {
            blocked = true;
            status = hr_mutex_recompute_owner_priority(mutex);
            if (status != HR_OK)
            {
                (void)hr_kernel_unblock_waiter(&current->wait_node,
                                               status,
                                               NULL);
            }
        }
    }

    hr_port_exit_critical(irq_state);

    if (blocked)
    {
        hr_port_request_context_switch();
        return current->wait_result;
    }

    return status;
}

hr_status_t hr_mutex_unlock(hr_mutex_t *mutex_object)
{
    hr_mutex_control_block_t *mutex;
    hr_task_t *current_task;
    hr_task_control_block_t *old_owner = NULL;
    hr_task_control_block_t *new_owner = NULL;
    hr_irq_state_t irq_state;
    hr_status_t status;
    bool higher_priority_task_woken = false;
    bool request_switch = false;

    if (!hr_mutex_is_valid(mutex_object))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (hr_port_is_inside_isr())
    {
        return HR_ERROR_FROM_ISR;
    }

    current_task = hr_task_current();
    if (!hr_task_is_valid(current_task))
    {
        return HR_ERROR_INVALID_STATE;
    }

    mutex = hr_mutex_control_block(mutex_object);
    irq_state = hr_port_enter_critical();

    if (mutex->owner != current_task)
    {
        status = HR_ERROR_NOT_OWNER;
    }
    else if (mutex->recursive && (mutex->recursion_count > 1U))
    {
        mutex->recursion_count--;
        status = HR_OK;
    }
    else
    {
        hr_wait_node_t *waiter = hr_wait_list_peek(&mutex->waiters);
        hr_task_t *next_owner_task = NULL;

        if (waiter != NULL)
        {
            next_owner_task = (hr_task_t *)hr_list_node_owner(&waiter->node);
            if (!hr_task_is_valid(next_owner_task))
            {
                hr_port_exit_critical(irq_state);
                return HR_ERROR_INTERNAL;
            }
        }

        status = hr_mutex_detach_owner(mutex, &old_owner);
        if ((status == HR_OK) && (next_owner_task != NULL))
        {
            status = hr_mutex_attach_owner(mutex, next_owner_task);
            if (status == HR_OK)
            {
                new_owner = hr_task_control_block(next_owner_task);
                status = hr_kernel_unblock_waiter(waiter,
                                                  HR_OK,
                                                  &higher_priority_task_woken);
            }
        }

        if ((status == HR_OK) && (old_owner != NULL))
        {
            status = hr_mutex_recompute_task_priority(old_owner, 0U);
        }
        if ((status == HR_OK) && (new_owner != NULL))
        {
            status = hr_mutex_recompute_task_priority(new_owner, 0U);
        }

        request_switch = higher_priority_task_woken ||
                         hr_kernel_current_should_preempt();
    }

    hr_port_exit_critical(irq_state);

    if ((status == HR_OK) && request_switch)
    {
        hr_port_request_context_switch();
    }

    return status;
}
