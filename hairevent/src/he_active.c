#include <stddef.h>

#include "hairevent/he_active.h"
#include "hairtos/hr_queue.h"
#include "he_internal.h"

static void he_zero_bytes(void *memory, size_t size)
{
    unsigned char *bytes = (unsigned char *)memory;
    size_t index;

    for (index = 0U; index < size; index++)
    {
        bytes[index] = 0U;
    }
}

he_active_control_block_t *he_active_control_block(he_active_t *active)
{
    return (active == NULL) ? NULL :
           (he_active_control_block_t *)(void *)active->storage;
}

const he_active_control_block_t *he_active_control_block_const(
    const he_active_t *active)
{
    return (active == NULL) ? NULL :
           (const he_active_control_block_t *)(const void *)active->storage;
}

bool he_active_is_valid(const he_active_t *active)
{
    const he_active_control_block_t *control_block =
        he_active_control_block_const(active);

    return (control_block != NULL) &&
           (control_block->magic == HE_CFG_ACTIVE_MAGIC) &&
           (control_block->name != NULL) &&
           hr_task_is_valid(&control_block->task) &&
           hr_queue_is_valid(&control_block->queue) &&
           he_state_machine_is_valid(&control_block->state_machine);
}

static void he_active_task_entry(void *argument)
{
    he_active_t *active = (he_active_t *)argument;
    he_active_control_block_t *control_block = he_active_control_block(active);

    if (!he_active_is_valid(active) ||
        (he_state_machine_start(&control_block->state_machine) != HR_OK))
    {
        for (;;)
        {
            hr_task_yield();
        }
    }

    for (;;)
    {
        he_event_t *event = NULL;
        hr_status_t status = hr_queue_receive(&control_block->queue,
                                              &event,
                                              HR_WAIT_FOREVER);
        if ((status != HR_OK) || !he_event_is_valid(event))
        {
            for (;;)
            {
                hr_task_yield();
            }
        }

        status = he_state_machine_dispatch(&control_block->state_machine, event);
        (void)he_event_release(event);
        if (status != HR_OK)
        {
            for (;;)
            {
                hr_task_yield();
            }
        }
    }
}

hr_status_t he_active_create_static(he_active_t *active,
                                    const char *name,
                                    he_state_handler_t initial_state,
                                    void *context,
                                    he_event_t **queue_storage,
                                    size_t queue_capacity,
                                    hr_stack_t *stack,
                                    size_t stack_words,
                                    hr_priority_t priority)
{
    he_active_control_block_t *control_block;
    hr_status_t status;

    if ((active == NULL) || (name == NULL) || (initial_state == NULL) ||
        (queue_storage == NULL) || (queue_capacity == 0U) ||
        (stack == NULL))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (he_active_is_valid(active))
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = he_active_control_block(active);
    he_zero_bytes(control_block, sizeof(*control_block));

    status = hr_queue_create_static(&control_block->queue,
                                    queue_storage,
                                    sizeof(queue_storage[0]),
                                    queue_capacity);
    if (status != HR_OK)
    {
        return status;
    }

    status = he_state_machine_init(&control_block->state_machine,
                                   initial_state,
                                   context);
    if (status != HR_OK)
    {
        return status;
    }

    control_block->name = name;
    control_block->magic = HE_CFG_ACTIVE_MAGIC;

    status = hr_task_create_static(&control_block->task,
                                   name,
                                   he_active_task_entry,
                                   active,
                                   stack,
                                   stack_words,
                                   priority);
    if (status != HR_OK)
    {
        control_block->magic = 0U;
        return status;
    }

    status = hr_task_start(&control_block->task);
    if (status != HR_OK)
    {
        control_block->magic = 0U;
        return status;
    }

    return HR_OK;
}

hr_status_t he_active_post(he_active_t *active,
                           he_event_t *event,
                           hr_tick_t timeout)
{
    he_active_control_block_t *control_block;

    if (!he_active_is_valid(active) || !he_event_is_valid(event))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    control_block = he_active_control_block(active);
    return hr_queue_send(&control_block->queue, &event, timeout);
}

hr_status_t he_active_post_shared(he_active_t *active,
                                  he_event_t *event,
                                  hr_tick_t timeout)
{
    hr_status_t status;

    status = he_event_retain(event);
    if (status != HR_OK)
    {
        return status;
    }

    status = he_active_post(active, event, timeout);
    if (status != HR_OK)
    {
        (void)he_event_release(event);
    }
    return status;
}

hr_status_t he_active_post_from_isr(he_active_t *active,
                                    he_event_t *event,
                                    bool *higher_priority_task_woken)
{
    he_active_control_block_t *control_block;

    if (!he_active_is_valid(active) || !he_event_is_valid(event))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    control_block = he_active_control_block(active);
    return hr_queue_send_from_isr(&control_block->queue,
                                  &event,
                                  higher_priority_task_woken);
}

const char *he_active_get_name(const he_active_t *active)
{
    const he_active_control_block_t *control_block =
        he_active_control_block_const(active);

    return he_active_is_valid(active) ? control_block->name : NULL;
}

size_t he_active_get_pending_count(const he_active_t *active)
{
    const he_active_control_block_t *control_block =
        he_active_control_block_const(active);

    return he_active_is_valid(active) ?
           hr_queue_get_count(&control_block->queue) : 0U;
}

hr_task_t *he_active_get_task(he_active_t *active)
{
    he_active_control_block_t *control_block = he_active_control_block(active);

    return he_active_is_valid(active) ? &control_block->task : NULL;
}

he_state_machine_t *he_active_get_state_machine(he_active_t *active)
{
    he_active_control_block_t *control_block = he_active_control_block(active);

    return he_active_is_valid(active) ? &control_block->state_machine : NULL;
}
