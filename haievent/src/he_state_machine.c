#include <stddef.h>

#include "haievent/he_state_machine.h"
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

he_state_machine_control_block_t *he_state_machine_control_block(
    he_state_machine_t *machine)
{
    return (machine == NULL) ? NULL :
           (he_state_machine_control_block_t *)(void *)machine->storage;
}

const he_state_machine_control_block_t *he_state_machine_control_block_const(
    const he_state_machine_t *machine)
{
    return (machine == NULL) ? NULL :
           (const he_state_machine_control_block_t *)(const void *)machine->storage;
}

bool he_state_machine_is_valid(const he_state_machine_t *machine)
{
    const he_state_machine_control_block_t *control_block =
        he_state_machine_control_block_const(machine);

    return (control_block != NULL) &&
           (control_block->magic == HE_CFG_STATE_MACHINE_MAGIC) &&
           (control_block->initial != NULL);
}

hr_status_t he_state_machine_init(he_state_machine_t *machine,
                                  he_state_handler_t initial_state,
                                  void *context)
{
    he_state_machine_control_block_t *control_block;

    if ((machine == NULL) || (initial_state == NULL))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (he_state_machine_is_valid(machine))
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block = he_state_machine_control_block(machine);
    he_zero_bytes(control_block, sizeof(*control_block));
    control_block->initial = initial_state;
    control_block->current = initial_state;
    control_block->context = context;
    control_block->magic = HE_CFG_STATE_MACHINE_MAGIC;
    return HR_OK;
}

bool he_state_machine_is_started(const he_state_machine_t *machine)
{
    const he_state_machine_control_block_t *control_block =
        he_state_machine_control_block_const(machine);

    return he_state_machine_is_valid(machine) && control_block->started;
}

he_state_handler_t he_state_machine_current(const he_state_machine_t *machine)
{
    const he_state_machine_control_block_t *control_block =
        he_state_machine_control_block_const(machine);

    return he_state_machine_is_valid(machine) ? control_block->current : NULL;
}

void *he_state_machine_context(he_state_machine_t *machine)
{
    he_state_machine_control_block_t *control_block =
        he_state_machine_control_block(machine);

    return he_state_machine_is_valid(machine) ? control_block->context : NULL;
}

const void *he_state_machine_context_const(const he_state_machine_t *machine)
{
    const he_state_machine_control_block_t *control_block =
        he_state_machine_control_block_const(machine);

    return he_state_machine_is_valid(machine) ? control_block->context : NULL;
}

he_state_result_t he_state_transition(he_state_machine_t *machine,
                                      he_state_handler_t target_state)
{
    he_state_machine_control_block_t *control_block =
        he_state_machine_control_block(machine);

    if (!he_state_machine_is_valid(machine) || (target_state == NULL))
    {
        return HE_STATE_IGNORED;
    }

    control_block->target = target_state;
    return HE_STATE_TRANSITION;
}

static hr_status_t he_dispatch_reserved(he_state_machine_t *machine,
                                        he_signal_t signal,
                                        he_state_result_t *result)
{
    he_state_machine_control_block_t *control_block =
        he_state_machine_control_block(machine);
    he_event_t event;

    if (he_event_init_static(&event, signal, sizeof(event)) != HR_OK)
    {
        return HR_ERROR_INTERNAL;
    }

    control_block->target = NULL;
    *result = control_block->current(machine, &event);
    return HR_OK;
}

static hr_status_t he_apply_transition(he_state_machine_t *machine)
{
    he_state_machine_control_block_t *control_block =
        he_state_machine_control_block(machine);
    he_state_result_t result;
    unsigned int transition_count = 0U;

    while (control_block->target != NULL)
    {
        he_state_handler_t target = control_block->target;
        control_block->target = NULL;

        if (he_dispatch_reserved(machine, (he_signal_t)HE_SIG_EXIT, &result) != HR_OK)
        {
            return HR_ERROR_INTERNAL;
        }
        (void)result;

        control_block->current = target;
        if (he_dispatch_reserved(machine, (he_signal_t)HE_SIG_ENTRY, &result) != HR_OK)
        {
            return HR_ERROR_INTERNAL;
        }
        if ((result == HE_STATE_TRANSITION) && (control_block->target == NULL))
        {
            return HR_ERROR_INVALID_STATE;
        }

        if (control_block->target == NULL)
        {
            if (he_dispatch_reserved(machine, (he_signal_t)HE_SIG_INIT, &result) != HR_OK)
            {
                return HR_ERROR_INTERNAL;
            }
            if ((result == HE_STATE_TRANSITION) && (control_block->target == NULL))
            {
                return HR_ERROR_INVALID_STATE;
            }
        }

        transition_count++;
        if (transition_count > HE_CFG_MAX_INIT_TRANSITIONS)
        {
            return HR_ERROR_OVERFLOW;
        }
    }

    return HR_OK;
}

hr_status_t he_state_machine_start(he_state_machine_t *machine)
{
    he_state_machine_control_block_t *control_block;
    he_state_result_t result;

    if (!he_state_machine_is_valid(machine))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    control_block = he_state_machine_control_block(machine);
    if (control_block->started)
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block->current = control_block->initial;
    control_block->started = true;

    if (he_dispatch_reserved(machine, (he_signal_t)HE_SIG_ENTRY, &result) != HR_OK)
    {
        return HR_ERROR_INTERNAL;
    }
    if ((result == HE_STATE_TRANSITION) && (control_block->target == NULL))
    {
        return HR_ERROR_INVALID_STATE;
    }

    if (control_block->target == NULL)
    {
        if (he_dispatch_reserved(machine, (he_signal_t)HE_SIG_INIT, &result) != HR_OK)
        {
            return HR_ERROR_INTERNAL;
        }
        if ((result == HE_STATE_TRANSITION) && (control_block->target == NULL))
        {
            return HR_ERROR_INVALID_STATE;
        }
    }

    return he_apply_transition(machine);
}

hr_status_t he_state_machine_dispatch(he_state_machine_t *machine,
                                      const he_event_t *event)
{
    he_state_machine_control_block_t *control_block;
    he_state_result_t result;

    if (!he_state_machine_is_valid(machine) ||
        !he_state_machine_is_started(machine) || !he_event_is_valid(event))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    control_block = he_state_machine_control_block(machine);
    control_block->target = NULL;
    result = control_block->current(machine, event);

    if (result == HE_STATE_TRANSITION)
    {
        if (control_block->target == NULL)
        {
            return HR_ERROR_INVALID_STATE;
        }
        return he_apply_transition(machine);
    }

    return ((result == HE_STATE_HANDLED) || (result == HE_STATE_IGNORED)) ?
           HR_OK : HR_ERROR_INVALID_STATE;
}
