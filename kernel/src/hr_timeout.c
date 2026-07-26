#include "hr_timeout_internal.h"

#include <stddef.h>

#define HR_CONTAINER_OF(pointer, type, member) \
    ((type *)((unsigned char *)(pointer) - offsetof(type, member)))

static hr_status_t hr_timeout_insert_sorted(hr_list_t *list, hr_timeout_node_t *node)
{
    hr_list_node_t *position = hr_list_front(list);

    while (position != NULL)
    {
        hr_timeout_node_t *current = HR_CONTAINER_OF(position, hr_timeout_node_t, node);

        if (node->wake_tick < current->wake_tick)
        {
            return hr_list_insert_before(list, position, &node->node);
        }

        position = hr_list_next(list, position);
    }

    return hr_list_push_back(list, &node->node);
}

static bool hr_timeout_order_is_valid(const hr_list_t *list)
{
    hr_list_node_t *position = hr_list_front(list);
    hr_tick_t previous = 0U;
    bool first = true;

    while (position != NULL)
    {
        hr_timeout_node_t *current = HR_CONTAINER_OF(position, hr_timeout_node_t, node);

        if (!first && (current->wake_tick < previous))
        {
            return false;
        }

        previous = current->wake_tick;
        first = false;
        position = hr_list_next(list, position);
    }

    return true;
}

void hr_timeout_node_init(hr_timeout_node_t *node, void *owner)
{
    if (node == NULL)
    {
        return;
    }

    hr_list_node_init(&node->node, owner);
    node->wake_tick = 0U;
}

void hr_timeout_list_init(hr_timeout_list_t *list, hr_tick_t start_tick)
{
    if (list == NULL)
    {
        return;
    }

    hr_list_init(&list->lists[0]);
    hr_list_init(&list->lists[1]);
    list->current = &list->lists[0];
    list->overflow = &list->lists[1];
    list->last_tick = start_tick;
    list->count = 0U;
}

bool hr_timeout_list_is_empty(const hr_timeout_list_t *list)
{
    return (list == NULL) || (list->count == 0U);
}

size_t hr_timeout_list_size(const hr_timeout_list_t *list)
{
    return (list == NULL) ? 0U : list->count;
}

hr_tick_t hr_timeout_list_last_tick(const hr_timeout_list_t *list)
{
    return (list == NULL) ? 0U : list->last_tick;
}

hr_status_t hr_timeout_list_insert(hr_timeout_list_t *list,
                                   hr_timeout_node_t *node,
                                   hr_tick_t delay_ticks)
{
    hr_tick_t wake_tick;
    hr_list_t *target;
    hr_status_t status;

    if ((list == NULL) || (node == NULL) || (delay_ticks == HR_WAIT_FOREVER))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (hr_list_node_is_linked(&node->node))
    {
        return HR_ERROR_INVALID_STATE;
    }

    wake_tick = list->last_tick + delay_ticks;
    node->wake_tick = wake_tick;
    target = (wake_tick < list->last_tick) ? list->overflow : list->current;

    status = hr_timeout_insert_sorted(target, node);
    if (status == HR_OK)
    {
        list->count++;
    }

    return status;
}

hr_status_t hr_timeout_list_remove(hr_timeout_list_t *list,
                                   hr_timeout_node_t *node)
{
    hr_status_t status;

    if ((list == NULL) || (node == NULL))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if ((node->node.list != list->current) && (node->node.list != list->overflow))
    {
        return HR_ERROR_INVALID_STATE;
    }

    status = hr_list_remove(&node->node);
    if (status == HR_OK)
    {
        list->count--;
    }

    return status;
}

hr_status_t hr_timeout_list_advance(hr_timeout_list_t *list,
                                    hr_tick_t now,
                                    hr_list_t *expired_nodes)
{
    hr_list_node_t *front;

    if ((list == NULL) || (expired_nodes == NULL))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (!hr_list_is_empty(expired_nodes))
    {
        return HR_ERROR_INVALID_STATE;
    }

    if (now < list->last_tick)
    {
        hr_list_t *temporary = list->current;
        list->current = list->overflow;
        list->overflow = temporary;
    }

    list->last_tick = now;
    front = hr_list_front(list->current);

    while (front != NULL)
    {
        hr_timeout_node_t *timeout = HR_CONTAINER_OF(front, hr_timeout_node_t, node);

        if (timeout->wake_tick > now)
        {
            break;
        }

        front = hr_list_next(list->current, front);
        if (hr_list_remove(&timeout->node) != HR_OK)
        {
            return HR_ERROR_INTERNAL;
        }
        if (hr_list_push_back(expired_nodes, &timeout->node) != HR_OK)
        {
            return HR_ERROR_INTERNAL;
        }
        list->count--;
    }

    return HR_OK;
}

bool hr_timeout_list_validate(const hr_timeout_list_t *list)
{
    size_t total;

    if ((list == NULL) || (list->current == NULL) || (list->overflow == NULL) ||
        (list->current == list->overflow))
    {
        return false;
    }

    if (!hr_list_validate(list->current) || !hr_list_validate(list->overflow))
    {
        return false;
    }

    if (!hr_timeout_order_is_valid(list->current) ||
        !hr_timeout_order_is_valid(list->overflow))
    {
        return false;
    }

    total = hr_list_size(list->current) + hr_list_size(list->overflow);
    return total == list->count;
}
