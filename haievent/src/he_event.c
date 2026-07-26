#include <limits.h>
#include <stdint.h>

#include "haievent/he_event.h"
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

he_event_pool_control_block_t *he_event_pool_control_block(he_event_pool_t *pool)
{
    return (pool == NULL) ? NULL :
           (he_event_pool_control_block_t *)(void *)pool->storage;
}

const he_event_pool_control_block_t *he_event_pool_control_block_const(
    const he_event_pool_t *pool)
{
    return (pool == NULL) ? NULL :
           (const he_event_pool_control_block_t *)(const void *)pool->storage;
}

bool he_event_pool_is_valid(const he_event_pool_t *pool)
{
    const he_event_pool_control_block_t *control_block =
        he_event_pool_control_block_const(pool);

    return (control_block != NULL) &&
           (control_block->magic == HE_CFG_EVENT_POOL_MAGIC) &&
           (control_block->storage != NULL) &&
           (control_block->block_size >= sizeof(he_event_t)) &&
           (control_block->block_count > 0U) &&
           (control_block->free_count <= control_block->block_count);
}

hr_status_t he_event_pool_init(he_event_pool_t *pool,
                               void *storage,
                               size_t block_size,
                               size_t block_count)
{
    he_event_pool_control_block_t *control_block;
    unsigned char *bytes;
    size_t index;

    if ((pool == NULL) || (storage == NULL) ||
        (block_size < sizeof(he_event_t)) || (block_count == 0U) ||
        (block_size > (size_t)UINT16_MAX) ||
        ((block_size % _Alignof(max_align_t)) != 0U) ||
        (((uintptr_t)storage % _Alignof(max_align_t)) != 0U))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (he_event_pool_is_valid(pool))
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = he_event_pool_control_block(pool);
    he_zero_bytes(control_block, sizeof(*control_block));
    bytes = (unsigned char *)storage;

    for (index = 0U; index < block_count; index++)
    {
        void **next = (void **)(void *)&bytes[index * block_size];
        *next = (index + 1U < block_count) ?
                (void *)&bytes[(index + 1U) * block_size] : NULL;
    }

    control_block->storage = bytes;
    control_block->free_head = storage;
    control_block->block_size = block_size;
    control_block->block_count = block_count;
    control_block->free_count = block_count;
    control_block->magic = HE_CFG_EVENT_POOL_MAGIC;
    return HR_OK;
}

size_t he_event_pool_get_free_count(const he_event_pool_t *pool)
{
    const he_event_pool_control_block_t *control_block =
        he_event_pool_control_block_const(pool);

    return he_event_pool_is_valid(pool) ? control_block->free_count : 0U;
}

size_t he_event_pool_get_block_count(const he_event_pool_t *pool)
{
    const he_event_pool_control_block_t *control_block =
        he_event_pool_control_block_const(pool);

    return he_event_pool_is_valid(pool) ? control_block->block_count : 0U;
}

he_event_t *he_event_new(he_event_pool_t *pool,
                         he_signal_t signal,
                         size_t event_size)
{
    he_event_pool_control_block_t *control_block;
    he_event_t *event;
    hr_irq_state_t irq_state;

    if (!he_event_pool_is_valid(pool) ||
        (signal == (he_signal_t)HE_SIG_NONE) ||
        (event_size < sizeof(he_event_t)))
    {
        return NULL;
    }

    control_block = he_event_pool_control_block(pool);
    if (event_size > control_block->block_size)
    {
        return NULL;
    }

    irq_state = hr_critical_enter();
    event = (he_event_t *)control_block->free_head;
    if (event != NULL)
    {
        control_block->free_head = *(void **)(void *)event;
        control_block->free_count--;
    }
    hr_critical_exit(irq_state);

    if (event == NULL)
    {
        return NULL;
    }

    he_zero_bytes(event, control_block->block_size);
    event->pool = pool;
    event->magic = HE_CFG_EVENT_MAGIC;
    event->size = (uint16_t)event_size;
    event->reference_count = 1U;
    event->signal = signal;
    event->flags = HE_EVENT_FLAG_DYNAMIC;
    return event;
}

hr_status_t he_event_init_static(he_event_t *event,
                                 he_signal_t signal,
                                 size_t event_size)
{
    if ((event == NULL) || (signal == (he_signal_t)HE_SIG_NONE) ||
        (event_size < sizeof(*event)) || (event_size > (size_t)UINT16_MAX))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    he_zero_bytes(event, sizeof(*event));
    event->pool = NULL;
    event->magic = HE_CFG_EVENT_MAGIC;
    event->size = (uint16_t)event_size;
    event->reference_count = 0U;
    event->signal = signal;
    event->flags = HE_EVENT_FLAG_STATIC;
    return HR_OK;
}

bool he_event_is_valid(const he_event_t *event)
{
    if ((event == NULL) || (event->magic != HE_CFG_EVENT_MAGIC) ||
        (event->signal == (he_signal_t)HE_SIG_NONE) ||
        (event->size < sizeof(*event)))
    {
        return false;
    }

    if (event->flags == HE_EVENT_FLAG_STATIC)
    {
        return (event->pool == NULL) && (event->reference_count == 0U);
    }

    if (event->flags == HE_EVENT_FLAG_DYNAMIC)
    {
        return (event->pool != NULL) && (event->reference_count > 0U) &&
               he_event_pool_is_valid((const he_event_pool_t *)event->pool);
    }

    return false;
}

bool he_event_is_dynamic(const he_event_t *event)
{
    return he_event_is_valid(event) &&
           (event->flags == HE_EVENT_FLAG_DYNAMIC);
}

uint16_t he_event_get_reference_count(const he_event_t *event)
{
    return he_event_is_dynamic(event) ? event->reference_count : 0U;
}

hr_status_t he_event_retain(he_event_t *event)
{
    hr_irq_state_t irq_state;

    if (!he_event_is_valid(event))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (!he_event_is_dynamic(event))
    {
        return HR_OK;
    }

    irq_state = hr_critical_enter();
    if (event->reference_count == UINT16_MAX)
    {
        hr_critical_exit(irq_state);
        return HR_ERROR_OVERFLOW;
    }
    event->reference_count++;
    hr_critical_exit(irq_state);
    return HR_OK;
}

hr_status_t he_event_release(he_event_t *event)
{
    he_event_pool_t *pool;
    he_event_pool_control_block_t *control_block;
    hr_irq_state_t irq_state;

    if (!he_event_is_valid(event))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (!he_event_is_dynamic(event))
    {
        return HR_OK;
    }

    pool = (he_event_pool_t *)event->pool;
    control_block = he_event_pool_control_block(pool);
    irq_state = hr_critical_enter();

    if (event->reference_count == 0U)
    {
        hr_critical_exit(irq_state);
        return HR_ERROR_INVALID_STATE;
    }

    event->reference_count--;
    if (event->reference_count == 0U)
    {
        if (control_block->free_count >= control_block->block_count)
        {
            hr_critical_exit(irq_state);
            return HR_ERROR_INTERNAL;
        }

        event->magic = 0U;
        event->pool = NULL;
        *(void **)(void *)event = control_block->free_head;
        control_block->free_head = event;
        control_block->free_count++;
    }

    hr_critical_exit(irq_state);
    return HR_OK;
}
