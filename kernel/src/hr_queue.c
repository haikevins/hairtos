#include <stdint.h>

#include "hairtos_config.h"
#include "hairtos/hr_queue.h"
#include "hr_kernel_internal.h"
#include "hr_port.h"
#include "hr_queue_internal.h"
#include "hr_task_internal.h"

static void hr_queue_zero_control_block(hr_queue_control_block_t *control_block)
{
    unsigned char *bytes = (unsigned char *)control_block;
    size_t index;

    for (index = 0U; index < sizeof(*control_block); index++)
    {
        bytes[index] = 0U;
    }
}

static void hr_queue_copy(void *destination, const void *source, size_t size)
{
    unsigned char *destination_bytes = (unsigned char *)destination;
    const unsigned char *source_bytes = (const unsigned char *)source;
    size_t index;

    for (index = 0U; index < size; index++)
    {
        destination_bytes[index] = source_bytes[index];
    }
}

static unsigned char *hr_queue_slot(hr_queue_control_block_t *control_block,
                                    size_t index)
{
    return &control_block->storage[index * control_block->item_size];
}

static hr_task_control_block_t *hr_queue_waiter_task(hr_wait_node_t *wait_node)
{
    hr_task_t *task;

    if (wait_node == NULL)
    {
        return NULL;
    }

    task = (hr_task_t *)hr_list_node_owner(&wait_node->node);
    return hr_task_is_valid(task) ? hr_task_control_block(task) : NULL;
}

static bool hr_queue_multiply_fits(size_t left, size_t right)
{
    return (left == 0U) || (right <= (SIZE_MAX / left));
}

hr_queue_control_block_t *hr_queue_control_block(hr_queue_t *queue)
{
    return (queue == NULL) ? NULL :
           (hr_queue_control_block_t *)(void *)queue->storage;
}

const hr_queue_control_block_t *hr_queue_control_block_const(const hr_queue_t *queue)
{
    return (queue == NULL) ? NULL :
           (const hr_queue_control_block_t *)(const void *)queue->storage;
}

bool hr_queue_is_valid(const hr_queue_t *queue)
{
    const hr_queue_control_block_t *control_block =
        hr_queue_control_block_const(queue);

    return (control_block != NULL) &&
           (control_block->magic == HR_CFG_QUEUE_MAGIC) &&
           (control_block->storage != NULL) &&
           (control_block->item_size > 0U) &&
           (control_block->capacity > 0U) &&
           (control_block->count <= control_block->capacity) &&
           (control_block->head < control_block->capacity) &&
           (control_block->tail < control_block->capacity);
}

bool hr_queue_validate_internal(const hr_queue_t *queue)
{
    const hr_queue_control_block_t *control_block =
        hr_queue_control_block_const(queue);

    return hr_queue_is_valid(queue) &&
           hr_wait_list_validate(&control_block->send_waiters) &&
           hr_wait_list_validate(&control_block->receive_waiters);
}

hr_status_t hr_queue_create_static(hr_queue_t *queue,
                                   void *storage,
                                   size_t item_size,
                                   size_t capacity)
{
    hr_queue_control_block_t *control_block;

    if ((queue == NULL) || (storage == NULL) ||
        (item_size == 0U) || (capacity == 0U) ||
        !hr_queue_multiply_fits(item_size, capacity))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (hr_queue_is_valid(queue))
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = hr_queue_control_block(queue);
    hr_queue_zero_control_block(control_block);
    control_block->storage = (unsigned char *)storage;
    control_block->item_size = item_size;
    control_block->capacity = capacity;
    hr_wait_list_init(&control_block->send_waiters);
    hr_wait_list_init(&control_block->receive_waiters);
    control_block->magic = HR_CFG_QUEUE_MAGIC;
    return HR_OK;
}

size_t hr_queue_get_count(const hr_queue_t *queue)
{
    const hr_queue_control_block_t *control_block =
        hr_queue_control_block_const(queue);

    return hr_queue_is_valid(queue) ? control_block->count : 0U;
}

size_t hr_queue_get_capacity(const hr_queue_t *queue)
{
    const hr_queue_control_block_t *control_block =
        hr_queue_control_block_const(queue);

    return hr_queue_is_valid(queue) ? control_block->capacity : 0U;
}

size_t hr_queue_get_waiting_senders(const hr_queue_t *queue)
{
    const hr_queue_control_block_t *control_block =
        hr_queue_control_block_const(queue);

    return hr_queue_is_valid(queue) ?
           hr_wait_list_size(&control_block->send_waiters) : 0U;
}

size_t hr_queue_get_waiting_receivers(const hr_queue_t *queue)
{
    const hr_queue_control_block_t *control_block =
        hr_queue_control_block_const(queue);

    return hr_queue_is_valid(queue) ?
           hr_wait_list_size(&control_block->receive_waiters) : 0U;
}

static hr_status_t hr_queue_wake_receiver(hr_queue_control_block_t *queue,
                                          const void *item,
                                          bool *higher_priority_task_woken)
{
    hr_wait_node_t *wait_node = hr_wait_list_peek(&queue->receive_waiters);
    hr_task_control_block_t *task = hr_queue_waiter_task(wait_node);

    if ((wait_node == NULL) || (task == NULL) ||
        (task->wait_kind != HR_TASK_WAIT_QUEUE_RECEIVE) ||
        (task->waiting_object != queue) || (task->wait_buffer == NULL))
    {
        return HR_ERROR_INTERNAL;
    }

    hr_queue_copy(task->wait_buffer, item, queue->item_size);
    return hr_kernel_unblock_waiter(wait_node,
                                    HR_OK,
                                    higher_priority_task_woken);
}

static hr_status_t hr_queue_fill_from_waiting_sender(
    hr_queue_control_block_t *queue,
    bool *higher_priority_task_woken)
{
    hr_wait_node_t *wait_node = hr_wait_list_peek(&queue->send_waiters);
    hr_task_control_block_t *task = hr_queue_waiter_task(wait_node);

    if (wait_node == NULL)
    {
        return HR_OK;
    }

    if ((task == NULL) || (task->wait_kind != HR_TASK_WAIT_QUEUE_SEND) ||
        (task->waiting_object != queue) || (task->wait_buffer == NULL) ||
        (queue->count >= queue->capacity))
    {
        return HR_ERROR_INTERNAL;
    }

    hr_queue_copy(hr_queue_slot(queue, queue->tail),
                  task->wait_buffer,
                  queue->item_size);
    queue->tail = (queue->tail + 1U) % queue->capacity;
    queue->count++;

    return hr_kernel_unblock_waiter(wait_node,
                                    HR_OK,
                                    higher_priority_task_woken);
}

static hr_status_t hr_queue_send_locked(hr_queue_t *queue_object,
                                        hr_queue_control_block_t *queue,
                                        const void *item,
                                        hr_tick_t timeout,
                                        bool allow_block,
                                        bool *blocked,
                                        bool *higher_priority_task_woken,
                                        hr_task_control_block_t **blocked_task)
{
    if (!hr_wait_list_is_empty(&queue->receive_waiters))
    {
        return hr_queue_wake_receiver(queue,
                                      item,
                                      higher_priority_task_woken);
    }

    if (queue->count < queue->capacity)
    {
        hr_queue_copy(hr_queue_slot(queue, queue->tail),
                      item,
                      queue->item_size);
        queue->tail = (queue->tail + 1U) % queue->capacity;
        queue->count++;
        return HR_OK;
    }

    if (timeout == HR_NO_WAIT)
    {
        return HR_ERROR_QUEUE_FULL;
    }

    if (!allow_block)
    {
        return HR_ERROR_INVALID_STATE;
    }

    *blocked_task = hr_task_control_block(hr_task_current());
    if (*blocked_task == NULL)
    {
        return HR_ERROR_INVALID_STATE;
    }

    {
        const hr_status_t block_status =
            hr_kernel_block_current_on_wait_list(&queue->send_waiters,
                                                 queue,
                                                 HR_TASK_WAIT_QUEUE_SEND,
                                                 (void *)item,
                                                 timeout);
        if (block_status != HR_OK)
        {
            return block_status;
        }
    }

    (void)queue_object;
    *blocked = true;
    return HR_OK;
}

static hr_status_t hr_queue_receive_locked(hr_queue_t *queue_object,
                                           hr_queue_control_block_t *queue,
                                           void *item,
                                           hr_tick_t timeout,
                                           bool allow_block,
                                           bool *blocked,
                                           bool *higher_priority_task_woken,
                                           hr_task_control_block_t **blocked_task)
{
    hr_status_t status;

    if (queue->count > 0U)
    {
        hr_queue_copy(item,
                      hr_queue_slot(queue, queue->head),
                      queue->item_size);
        queue->head = (queue->head + 1U) % queue->capacity;
        queue->count--;

        status = hr_queue_fill_from_waiting_sender(queue,
                                                   higher_priority_task_woken);
        return status;
    }

    if (!hr_wait_list_is_empty(&queue->send_waiters))
    {
        hr_wait_node_t *wait_node = hr_wait_list_peek(&queue->send_waiters);
        hr_task_control_block_t *task = hr_queue_waiter_task(wait_node);

        if ((task == NULL) || (task->wait_buffer == NULL))
        {
            return HR_ERROR_INTERNAL;
        }

        hr_queue_copy(item, task->wait_buffer, queue->item_size);
        return hr_kernel_unblock_waiter(wait_node,
                                        HR_OK,
                                        higher_priority_task_woken);
    }

    if (timeout == HR_NO_WAIT)
    {
        return HR_ERROR_QUEUE_EMPTY;
    }

    if (!allow_block)
    {
        return HR_ERROR_INVALID_STATE;
    }

    *blocked_task = hr_task_control_block(hr_task_current());
    if (*blocked_task == NULL)
    {
        return HR_ERROR_INVALID_STATE;
    }

    {
        const hr_status_t block_status =
            hr_kernel_block_current_on_wait_list(&queue->receive_waiters,
                                                 queue,
                                                 HR_TASK_WAIT_QUEUE_RECEIVE,
                                                 item,
                                                 timeout);
        if (block_status != HR_OK)
        {
            return block_status;
        }
    }

    (void)queue_object;
    *blocked = true;
    return HR_OK;
}

hr_status_t hr_queue_send(hr_queue_t *queue, const void *item, hr_tick_t timeout)
{
    hr_queue_control_block_t *control_block;
    hr_task_control_block_t *blocked_task = NULL;
    hr_irq_state_t irq_state;
    hr_status_t status;
    bool blocked = false;
    bool higher_priority_task_woken = false;

    if (!hr_queue_is_valid(queue) || (item == NULL))
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

    control_block = hr_queue_control_block(queue);
    irq_state = hr_port_enter_critical();
    status = hr_queue_send_locked(queue,
                                  control_block,
                                  item,
                                  timeout,
                                  true,
                                  &blocked,
                                  &higher_priority_task_woken,
                                  &blocked_task);
    hr_port_exit_critical(irq_state);

    if (blocked || higher_priority_task_woken)
    {
        hr_port_request_context_switch();
    }

    if (blocked)
    {
        return blocked_task->wait_result;
    }

    return status;
}

hr_status_t hr_queue_receive(hr_queue_t *queue, void *item, hr_tick_t timeout)
{
    hr_queue_control_block_t *control_block;
    hr_task_control_block_t *blocked_task = NULL;
    hr_irq_state_t irq_state;
    hr_status_t status;
    bool blocked = false;
    bool higher_priority_task_woken = false;

    if (!hr_queue_is_valid(queue) || (item == NULL))
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

    control_block = hr_queue_control_block(queue);
    irq_state = hr_port_enter_critical();
    status = hr_queue_receive_locked(queue,
                                     control_block,
                                     item,
                                     timeout,
                                     true,
                                     &blocked,
                                     &higher_priority_task_woken,
                                     &blocked_task);
    hr_port_exit_critical(irq_state);

    if (blocked || higher_priority_task_woken)
    {
        hr_port_request_context_switch();
    }

    if (blocked)
    {
        return blocked_task->wait_result;
    }

    return status;
}

hr_status_t hr_queue_send_from_isr(hr_queue_t *queue,
                                   const void *item,
                                   bool *higher_priority_task_woken)
{
    hr_queue_control_block_t *control_block;
    hr_task_control_block_t *unused_task = NULL;
    hr_irq_state_t irq_state;
    hr_status_t status;
    bool unused_blocked = false;
    bool switch_required = false;

    if (!hr_queue_is_valid(queue) || (item == NULL))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (!hr_port_is_inside_isr())
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = hr_queue_control_block(queue);
    irq_state = hr_port_enter_critical();
    status = hr_queue_send_locked(queue,
                                  control_block,
                                  item,
                                  HR_NO_WAIT,
                                  false,
                                  &unused_blocked,
                                  &switch_required,
                                  &unused_task);
    hr_port_exit_critical(irq_state);

    if (higher_priority_task_woken != NULL)
    {
        *higher_priority_task_woken = switch_required;
    }
    return status;
}

hr_status_t hr_queue_receive_from_isr(hr_queue_t *queue,
                                      void *item,
                                      bool *higher_priority_task_woken)
{
    hr_queue_control_block_t *control_block;
    hr_task_control_block_t *unused_task = NULL;
    hr_irq_state_t irq_state;
    hr_status_t status;
    bool unused_blocked = false;
    bool switch_required = false;

    if (!hr_queue_is_valid(queue) || (item == NULL))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (!hr_port_is_inside_isr())
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = hr_queue_control_block(queue);
    irq_state = hr_port_enter_critical();
    status = hr_queue_receive_locked(queue,
                                     control_block,
                                     item,
                                     HR_NO_WAIT,
                                     false,
                                     &unused_blocked,
                                     &switch_required,
                                     &unused_task);
    hr_port_exit_critical(irq_state);

    if (higher_priority_task_woken != NULL)
    {
        *higher_priority_task_woken = switch_required;
    }
    return status;
}
