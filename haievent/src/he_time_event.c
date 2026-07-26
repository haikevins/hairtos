#include <stddef.h>

#include "haievent/he_time_event.h"
#include "hairtos/hr_timer.h"
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

he_time_event_control_block_t *he_time_event_control_block(
    he_time_event_t *time_event)
{
    return (time_event == NULL) ? NULL :
           (he_time_event_control_block_t *)(void *)time_event->storage;
}

const he_time_event_control_block_t *he_time_event_control_block_const(
    const he_time_event_t *time_event)
{
    return (time_event == NULL) ? NULL :
           (const he_time_event_control_block_t *)(const void *)time_event->storage;
}

bool he_time_event_is_valid(const he_time_event_t *time_event)
{
    const he_time_event_control_block_t *control_block =
        he_time_event_control_block_const(time_event);

    return (control_block != NULL) &&
           (control_block->magic == HE_CFG_TIME_EVENT_MAGIC) &&
           (control_block->name != NULL) &&
           he_active_is_valid(control_block->target) &&
           he_event_is_valid(&control_block->event) &&
           hr_timer_is_valid(&control_block->timer);
}

static void he_time_event_callback(void *argument)
{
    he_time_event_t *time_event = (he_time_event_t *)argument;
    he_time_event_control_block_t *control_block =
        he_time_event_control_block(time_event);

    if (!he_time_event_is_valid(time_event))
    {
        return;
    }

    if (he_active_post(control_block->target,
                       &control_block->event,
                       HR_NO_WAIT) != HR_OK)
    {
        if (control_block->dropped_count < UINT32_MAX)
        {
            control_block->dropped_count++;
        }
    }
}

hr_status_t he_time_event_create_static(he_time_event_t *time_event,
                                        const char *name,
                                        he_active_t *target,
                                        he_signal_t signal,
                                        hr_tick_t period_ticks,
                                        bool periodic)
{
    he_time_event_control_block_t *control_block;
    hr_status_t status;

    if ((time_event == NULL) || (name == NULL) ||
        !he_active_is_valid(target) ||
        (signal < (he_signal_t)HE_SIG_USER) ||
        (period_ticks == 0U) || (period_ticks == HR_WAIT_FOREVER))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (he_time_event_is_valid(time_event))
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = he_time_event_control_block(time_event);
    he_zero_bytes(control_block, sizeof(*control_block));
    control_block->target = target;
    control_block->name = name;

    status = he_event_init_static(&control_block->event,
                                  signal,
                                  sizeof(control_block->event));
    if (status != HR_OK)
    {
        return status;
    }

    status = hr_timer_create_static(&control_block->timer,
                                    name,
                                    period_ticks,
                                    periodic,
                                    he_time_event_callback,
                                    time_event);
    if (status != HR_OK)
    {
        return status;
    }

    control_block->magic = HE_CFG_TIME_EVENT_MAGIC;
    return HR_OK;
}

hr_status_t he_time_event_arm(he_time_event_t *time_event)
{
    he_time_event_control_block_t *control_block =
        he_time_event_control_block(time_event);

    return he_time_event_is_valid(time_event) ?
           hr_timer_start(&control_block->timer) : HR_ERROR_INVALID_ARGUMENT;
}

hr_status_t he_time_event_disarm(he_time_event_t *time_event)
{
    he_time_event_control_block_t *control_block =
        he_time_event_control_block(time_event);

    return he_time_event_is_valid(time_event) ?
           hr_timer_stop(&control_block->timer) : HR_ERROR_INVALID_ARGUMENT;
}

hr_status_t he_time_event_rearm(he_time_event_t *time_event)
{
    he_time_event_control_block_t *control_block =
        he_time_event_control_block(time_event);

    return he_time_event_is_valid(time_event) ?
           hr_timer_reset(&control_block->timer) : HR_ERROR_INVALID_ARGUMENT;
}

hr_status_t he_time_event_change_period(he_time_event_t *time_event,
                                        hr_tick_t period_ticks)
{
    he_time_event_control_block_t *control_block =
        he_time_event_control_block(time_event);

    return he_time_event_is_valid(time_event) ?
           hr_timer_change_period(&control_block->timer, period_ticks) :
           HR_ERROR_INVALID_ARGUMENT;
}

bool he_time_event_is_armed(const he_time_event_t *time_event)
{
    const he_time_event_control_block_t *control_block =
        he_time_event_control_block_const(time_event);

    return he_time_event_is_valid(time_event) &&
           hr_timer_is_active(&control_block->timer);
}

uint32_t he_time_event_get_dropped_count(const he_time_event_t *time_event)
{
    const he_time_event_control_block_t *control_block =
        he_time_event_control_block_const(time_event);

    return he_time_event_is_valid(time_event) ?
           control_block->dropped_count : 0U;
}
