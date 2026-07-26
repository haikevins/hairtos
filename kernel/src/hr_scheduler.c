#include "hr_scheduler_internal.h"

#include <stddef.h>

#define HR_CONTAINER_OF(pointer, type, member) \
    ((type *)((unsigned char *)(pointer) - offsetof(type, member)))

static bool hr_priority_is_valid(hr_priority_t priority)
{
    return priority < (hr_priority_t)HR_CFG_PRIORITY_COUNT;
}

static uint32_t hr_priority_mask(hr_priority_t priority)
{
    return UINT32_C(1) << priority;
}

static hr_priority_t hr_ready_set_highest_priority(const hr_ready_set_t *set)
{
    hr_priority_t priority;

    for (priority = 0U; priority < (hr_priority_t)HR_CFG_PRIORITY_COUNT; priority++)
    {
        if ((set->bitmap & hr_priority_mask(priority)) != 0U)
        {
            return priority;
        }
    }

    return (hr_priority_t)HR_CFG_PRIORITY_COUNT;
}

void hr_ready_node_init(hr_ready_node_t *node, void *owner, hr_priority_t priority)
{
    if (node == NULL)
    {
        return;
    }

    hr_list_node_init(&node->node, owner);
    node->priority = priority;
}

void hr_ready_set_init(hr_ready_set_t *set)
{
    size_t index;

    if (set == NULL)
    {
        return;
    }

    for (index = 0U; index < (size_t)HR_CFG_PRIORITY_COUNT; index++)
    {
        hr_list_init(&set->queues[index]);
    }

    set->bitmap = 0U;
    set->count = 0U;
}

bool hr_ready_set_is_empty(const hr_ready_set_t *set)
{
    return (set == NULL) || (set->count == 0U);
}

size_t hr_ready_set_size(const hr_ready_set_t *set)
{
    return (set == NULL) ? 0U : set->count;
}

uint32_t hr_ready_set_bitmap(const hr_ready_set_t *set)
{
    return (set == NULL) ? 0U : set->bitmap;
}

hr_status_t hr_ready_set_insert(hr_ready_set_t *set, hr_ready_node_t *node)
{
    hr_status_t status;
    hr_list_t *queue;

    if ((set == NULL) || (node == NULL) || !hr_priority_is_valid(node->priority))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    queue = &set->queues[node->priority];
    status = hr_list_push_back(queue, &node->node);
    if (status != HR_OK)
    {
        return status;
    }

    set->bitmap |= hr_priority_mask(node->priority);
    set->count++;
    return HR_OK;
}

hr_status_t hr_ready_set_remove(hr_ready_set_t *set, hr_ready_node_t *node)
{
    hr_status_t status;
    hr_list_t *queue;

    if ((set == NULL) || (node == NULL) || !hr_priority_is_valid(node->priority))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    queue = &set->queues[node->priority];
    if (node->node.list != queue)
    {
        return HR_ERROR_INVALID_STATE;
    }

    status = hr_list_remove(&node->node);
    if (status != HR_OK)
    {
        return status;
    }

    set->count--;
    if (hr_list_is_empty(queue))
    {
        set->bitmap &= ~hr_priority_mask(node->priority);
    }

    return HR_OK;
}

hr_ready_node_t *hr_ready_set_peek_highest(const hr_ready_set_t *set)
{
    hr_priority_t priority;
    hr_list_node_t *node;

    if ((set == NULL) || (set->count == 0U))
    {
        return NULL;
    }

    priority = hr_ready_set_highest_priority(set);
    if (!hr_priority_is_valid(priority))
    {
        return NULL;
    }

    node = hr_list_front(&set->queues[priority]);
    return (node == NULL) ? NULL : HR_CONTAINER_OF(node, hr_ready_node_t, node);
}

hr_status_t hr_ready_set_rotate_highest(hr_ready_set_t *set)
{
    hr_priority_t priority;
    hr_list_t *queue;
    hr_list_node_t *node;
    hr_status_t status;

    if (set == NULL)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (set->count == 0U)
    {
        return HR_ERROR_INVALID_STATE;
    }

    priority = hr_ready_set_highest_priority(set);
    if (!hr_priority_is_valid(priority))
    {
        return HR_ERROR_INTERNAL;
    }

    queue = &set->queues[priority];
    if (hr_list_size(queue) < 2U)
    {
        return HR_OK;
    }

    node = hr_list_pop_front(queue);
    if (node == NULL)
    {
        return HR_ERROR_INTERNAL;
    }

    status = hr_list_push_back(queue, node);
    return status;
}

bool hr_ready_set_validate(const hr_ready_set_t *set)
{
    size_t index;
    size_t total = 0U;
    uint32_t expected_bitmap = 0U;

    if (set == NULL)
    {
        return false;
    }

    for (index = 0U; index < (size_t)HR_CFG_PRIORITY_COUNT; index++)
    {
        const hr_list_t *queue = &set->queues[index];

        if (!hr_list_validate(queue))
        {
            return false;
        }

        total += hr_list_size(queue);
        if (!hr_list_is_empty(queue))
        {
            expected_bitmap |= UINT32_C(1) << index;
        }
    }

    return (total == set->count) && (expected_bitmap == set->bitmap);
}

void hr_scheduler_init(hr_scheduler_t *scheduler)
{
    if (scheduler == NULL)
    {
        return;
    }

    hr_ready_set_init(&scheduler->ready);
}

hr_status_t hr_scheduler_add_ready(hr_scheduler_t *scheduler, hr_ready_node_t *node)
{
    if (scheduler == NULL)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    return hr_ready_set_insert(&scheduler->ready, node);
}

hr_status_t hr_scheduler_remove_ready(hr_scheduler_t *scheduler, hr_ready_node_t *node)
{
    if (scheduler == NULL)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    return hr_ready_set_remove(&scheduler->ready, node);
}

hr_ready_node_t *hr_scheduler_select_highest(const hr_scheduler_t *scheduler)
{
    return (scheduler == NULL) ? NULL : hr_ready_set_peek_highest(&scheduler->ready);
}

hr_status_t hr_scheduler_yield_current(hr_scheduler_t *scheduler,
                                       hr_ready_node_t *current)
{
    hr_ready_node_t *selected;

    if ((scheduler == NULL) || (current == NULL))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    selected = hr_scheduler_select_highest(scheduler);
    if (selected != current)
    {
        /* Fixed-priority policy never lets a non-highest task drive rotation. */
        return HR_ERROR_INVALID_STATE;
    }

    return hr_ready_set_rotate_highest(&scheduler->ready);
}

size_t hr_scheduler_ready_count(const hr_scheduler_t *scheduler)
{
    return (scheduler == NULL) ? 0U : hr_ready_set_size(&scheduler->ready);
}

uint32_t hr_scheduler_ready_bitmap(const hr_scheduler_t *scheduler)
{
    return (scheduler == NULL) ? 0U : hr_ready_set_bitmap(&scheduler->ready);
}

bool hr_scheduler_validate(const hr_scheduler_t *scheduler)
{
    return (scheduler != NULL) && hr_ready_set_validate(&scheduler->ready);
}
