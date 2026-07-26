#include "hr_wait_internal.h"

#include <stddef.h>

#define HR_CONTAINER_OF(pointer, type, member) \
    ((type *)((unsigned char *)(pointer) - offsetof(type, member)))

void hr_wait_node_init(hr_wait_node_t *node, void *owner, hr_priority_t priority)
{
    if (node == NULL)
    {
        return;
    }

    hr_list_node_init(&node->node, owner);
    node->priority = priority;
}

void hr_wait_list_init(hr_wait_list_t *list)
{
    if (list != NULL)
    {
        hr_list_init(&list->list);
    }
}

bool hr_wait_list_is_empty(const hr_wait_list_t *list)
{
    return (list == NULL) || hr_list_is_empty(&list->list);
}

size_t hr_wait_list_size(const hr_wait_list_t *list)
{
    return (list == NULL) ? 0U : hr_list_size(&list->list);
}

hr_status_t hr_wait_list_insert(hr_wait_list_t *list, hr_wait_node_t *node)
{
    hr_list_node_t *position;

    if ((list == NULL) || (node == NULL))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (hr_list_node_is_linked(&node->node))
    {
        return HR_ERROR_INVALID_STATE;
    }

    position = hr_list_front(&list->list);
    while (position != NULL)
    {
        hr_wait_node_t *current = HR_CONTAINER_OF(position, hr_wait_node_t, node);

        if (node->priority < current->priority)
        {
            return hr_list_insert_before(&list->list, position, &node->node);
        }

        position = hr_list_next(&list->list, position);
    }

    return hr_list_push_back(&list->list, &node->node);
}

hr_status_t hr_wait_list_remove(hr_wait_list_t *list, hr_wait_node_t *node)
{
    if ((list == NULL) || (node == NULL))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (node->node.list != &list->list)
    {
        return HR_ERROR_INVALID_STATE;
    }

    return hr_list_remove(&node->node);
}

hr_wait_node_t *hr_wait_list_peek(const hr_wait_list_t *list)
{
    hr_list_node_t *node;

    if (list == NULL)
    {
        return NULL;
    }

    node = hr_list_front(&list->list);
    return (node == NULL) ? NULL : HR_CONTAINER_OF(node, hr_wait_node_t, node);
}

hr_wait_node_t *hr_wait_list_pop(hr_wait_list_t *list)
{
    hr_list_node_t *node;

    if (list == NULL)
    {
        return NULL;
    }

    node = hr_list_pop_front(&list->list);
    return (node == NULL) ? NULL : HR_CONTAINER_OF(node, hr_wait_node_t, node);
}

bool hr_wait_list_validate(const hr_wait_list_t *list)
{
    hr_list_node_t *node;
    hr_priority_t previous_priority = 0U;
    bool first = true;

    if ((list == NULL) || !hr_list_validate(&list->list))
    {
        return false;
    }

    node = hr_list_front(&list->list);
    while (node != NULL)
    {
        hr_wait_node_t *current = HR_CONTAINER_OF(node, hr_wait_node_t, node);

        if (!first && (current->priority < previous_priority))
        {
            return false;
        }

        previous_priority = current->priority;
        first = false;
        node = hr_list_next(&list->list, node);
    }

    return true;
}
