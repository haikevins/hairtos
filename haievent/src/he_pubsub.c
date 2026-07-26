#include <stddef.h>

#include "haievent/he_pubsub.h"
#include "he_internal.h"
#include "hairtos/hr_context.h"

static void he_zero_bytes(void *memory, size_t size)
{
    unsigned char *bytes = (unsigned char *)memory;
    size_t index;

    for (index = 0U; index < size; index++)
    {
        bytes[index] = 0U;
    }
}

he_pubsub_control_block_t *he_pubsub_control_block(he_pubsub_t *pubsub)
{
    return (pubsub == NULL) ? NULL :
           (he_pubsub_control_block_t *)(void *)pubsub->storage;
}

const he_pubsub_control_block_t *he_pubsub_control_block_const(
    const he_pubsub_t *pubsub)
{
    return (pubsub == NULL) ? NULL :
           (const he_pubsub_control_block_t *)(const void *)pubsub->storage;
}

bool he_pubsub_is_valid(const he_pubsub_t *pubsub)
{
    const he_pubsub_control_block_t *control_block =
        he_pubsub_control_block_const(pubsub);

    return (control_block != NULL) &&
           (control_block->magic == HE_CFG_PUBSUB_MAGIC) &&
           (control_block->subscribers != NULL) &&
           (control_block->signal_count > (size_t)HE_SIG_USER) &&
           (control_block->max_subscribers > 0U) &&
           (control_block->max_subscribers <= HE_CFG_MAX_ACTIVE_OBJECTS);
}

hr_status_t he_pubsub_init(he_pubsub_t *pubsub,
                           he_active_t **subscriber_storage,
                           size_t signal_count,
                           size_t max_subscribers_per_signal)
{
    he_pubsub_control_block_t *control_block;

    if ((pubsub == NULL) || (subscriber_storage == NULL) ||
        (signal_count <= (size_t)HE_SIG_USER) ||
        (signal_count > HE_CFG_MAX_SIGNALS) ||
        (max_subscribers_per_signal == 0U) ||
        (max_subscribers_per_signal > HE_CFG_MAX_ACTIVE_OBJECTS))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (he_pubsub_is_valid(pubsub))
    {
        return HR_ERROR_INVALID_STATE;
    }

    he_zero_bytes(subscriber_storage,
                  signal_count * max_subscribers_per_signal *
                  sizeof(subscriber_storage[0]));
    control_block = he_pubsub_control_block(pubsub);
    he_zero_bytes(control_block, sizeof(*control_block));
    control_block->subscribers = subscriber_storage;
    control_block->signal_count = signal_count;
    control_block->max_subscribers = max_subscribers_per_signal;
    control_block->magic = HE_CFG_PUBSUB_MAGIC;
    return HR_OK;
}

static he_active_t **he_pubsub_signal_slots(he_pubsub_control_block_t *control_block,
                                            he_signal_t signal)
{
    return &control_block->subscribers[(size_t)signal *
                                       control_block->max_subscribers];
}

static he_active_t *const *he_pubsub_signal_slots_const(
    const he_pubsub_control_block_t *control_block,
    he_signal_t signal)
{
    return &control_block->subscribers[(size_t)signal *
                                       control_block->max_subscribers];
}

hr_status_t he_pubsub_subscribe(he_pubsub_t *pubsub,
                                he_signal_t signal,
                                he_active_t *subscriber)
{
    he_pubsub_control_block_t *control_block;
    he_active_t **slots;
    hr_irq_state_t irq_state;
    size_t index;

    if (!he_pubsub_is_valid(pubsub) || !he_active_is_valid(subscriber) ||
        (signal < (he_signal_t)HE_SIG_USER))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    control_block = he_pubsub_control_block(pubsub);
    if ((size_t)signal >= control_block->signal_count)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    irq_state = hr_critical_enter();
    slots = he_pubsub_signal_slots(control_block, signal);
    for (index = 0U; index < control_block->max_subscribers; index++)
    {
        if (slots[index] == subscriber)
        {
            hr_critical_exit(irq_state);
            return HR_ERROR_INVALID_STATE;
        }
    }
    for (index = 0U; index < control_block->max_subscribers; index++)
    {
        if (slots[index] == NULL)
        {
            slots[index] = subscriber;
            hr_critical_exit(irq_state);
            return HR_OK;
        }
    }
    hr_critical_exit(irq_state);
    return HR_ERROR_NO_MEMORY;
}

hr_status_t he_pubsub_unsubscribe(he_pubsub_t *pubsub,
                                  he_signal_t signal,
                                  he_active_t *subscriber)
{
    he_pubsub_control_block_t *control_block;
    he_active_t **slots;
    hr_irq_state_t irq_state;
    size_t index;

    if (!he_pubsub_is_valid(pubsub) || !he_active_is_valid(subscriber) ||
        (signal < (he_signal_t)HE_SIG_USER))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    control_block = he_pubsub_control_block(pubsub);
    if ((size_t)signal >= control_block->signal_count)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    irq_state = hr_critical_enter();
    slots = he_pubsub_signal_slots(control_block, signal);
    for (index = 0U; index < control_block->max_subscribers; index++)
    {
        if (slots[index] == subscriber)
        {
            size_t move_index;
            for (move_index = index;
                 move_index + 1U < control_block->max_subscribers;
                 move_index++)
            {
                slots[move_index] = slots[move_index + 1U];
            }
            slots[control_block->max_subscribers - 1U] = NULL;
            hr_critical_exit(irq_state);
            return HR_OK;
        }
    }
    hr_critical_exit(irq_state);
    return HR_ERROR_INVALID_STATE;
}

size_t he_pubsub_get_subscriber_count(const he_pubsub_t *pubsub,
                                      he_signal_t signal)
{
    const he_pubsub_control_block_t *control_block =
        he_pubsub_control_block_const(pubsub);
    he_active_t *const *slots;
    size_t count = 0U;
    size_t index;

    if (!he_pubsub_is_valid(pubsub) ||
        ((size_t)signal >= control_block->signal_count))
    {
        return 0U;
    }

    slots = he_pubsub_signal_slots_const(control_block, signal);
    for (index = 0U; index < control_block->max_subscribers; index++)
    {
        if (slots[index] != NULL)
        {
            count++;
        }
    }
    return count;
}

hr_status_t he_pubsub_publish(he_pubsub_t *pubsub,
                              he_event_t *event,
                              hr_tick_t timeout,
                              size_t *delivered_count)
{
    he_pubsub_control_block_t *control_block;
    he_active_t *snapshot[HE_CFG_MAX_ACTIVE_OBJECTS];
    hr_irq_state_t irq_state;
    hr_status_t overall_status = HR_OK;
    size_t snapshot_count = 0U;
    size_t delivered = 0U;
    size_t index;

    if (delivered_count != NULL)
    {
        *delivered_count = 0U;
    }

    if (!he_pubsub_is_valid(pubsub) || !he_event_is_valid(event) ||
        (event->signal < (he_signal_t)HE_SIG_USER))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    control_block = he_pubsub_control_block(pubsub);
    if ((size_t)event->signal >= control_block->signal_count)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    irq_state = hr_critical_enter();
    {
        he_active_t **slots = he_pubsub_signal_slots(control_block, event->signal);
        for (index = 0U; index < control_block->max_subscribers; index++)
        {
            if (slots[index] != NULL)
            {
                snapshot[snapshot_count] = slots[index];
                snapshot_count++;
            }
        }
    }
    hr_critical_exit(irq_state);

    for (index = 0U; index < snapshot_count; index++)
    {
        hr_status_t status = he_active_post_shared(snapshot[index], event, timeout);
        if (status == HR_OK)
        {
            delivered++;
        }
        else if (overall_status == HR_OK)
        {
            overall_status = status;
        }
    }

    if (he_event_is_dynamic(event))
    {
        hr_status_t release_status = he_event_release(event);
        if ((release_status != HR_OK) && (overall_status == HR_OK))
        {
            overall_status = release_status;
        }
    }

    if (delivered_count != NULL)
    {
        *delivered_count = delivered;
    }
    return overall_status;
}
