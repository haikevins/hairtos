#include "hr_list_internal.h"

static hr_status_t hr_list_insert_between(hr_list_t *list,
                                          hr_list_node_t *previous,
                                          hr_list_node_t *next,
                                          hr_list_node_t *node)
{
    if ((list == NULL) || (previous == NULL) || (next == NULL) || (node == NULL))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (node->list != NULL)
    {
        return HR_ERROR_INVALID_STATE;
    }

    node->previous = previous;
    node->next = next;
    node->list = list;
    previous->next = node;
    next->previous = node;
    list->size++;

    return HR_OK;
}

void hr_list_init(hr_list_t *list)
{
    if (list == NULL)
    {
        return;
    }

    list->root.previous = &list->root;
    list->root.next = &list->root;
    list->root.list = list;
    list->root.owner = NULL;
    list->size = 0U;
}

void hr_list_node_init(hr_list_node_t *node, void *owner)
{
    if (node == NULL)
    {
        return;
    }

    node->previous = NULL;
    node->next = NULL;
    node->list = NULL;
    node->owner = owner;
}

bool hr_list_is_empty(const hr_list_t *list)
{
    return (list == NULL) || (list->size == 0U);
}

size_t hr_list_size(const hr_list_t *list)
{
    return (list == NULL) ? 0U : list->size;
}

bool hr_list_node_is_linked(const hr_list_node_t *node)
{
    return (node != NULL) && (node->list != NULL);
}

void *hr_list_node_owner(const hr_list_node_t *node)
{
    return (node == NULL) ? NULL : node->owner;
}

hr_list_node_t *hr_list_front(const hr_list_t *list)
{
    if ((list == NULL) || (list->size == 0U))
    {
        return NULL;
    }

    return list->root.next;
}

hr_list_node_t *hr_list_back(const hr_list_t *list)
{
    if ((list == NULL) || (list->size == 0U))
    {
        return NULL;
    }

    return list->root.previous;
}

hr_list_node_t *hr_list_next(const hr_list_t *list, const hr_list_node_t *node)
{
    if ((list == NULL) || (node == NULL) || (node->list != list) ||
        (node->next == &list->root))
    {
        return NULL;
    }

    return node->next;
}

hr_list_node_t *hr_list_previous(const hr_list_t *list, const hr_list_node_t *node)
{
    if ((list == NULL) || (node == NULL) || (node->list != list) ||
        (node->previous == &list->root))
    {
        return NULL;
    }

    return node->previous;
}

hr_status_t hr_list_push_front(hr_list_t *list, hr_list_node_t *node)
{
    if (list == NULL)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    return hr_list_insert_between(list, &list->root, list->root.next, node);
}

hr_status_t hr_list_push_back(hr_list_t *list, hr_list_node_t *node)
{
    if (list == NULL)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    return hr_list_insert_between(list, list->root.previous, &list->root, node);
}

hr_status_t hr_list_insert_before(hr_list_t *list,
                                  hr_list_node_t *position,
                                  hr_list_node_t *node)
{
    if ((list == NULL) || (position == NULL))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    if (position->list != list)
    {
        return HR_ERROR_INVALID_STATE;
    }

    return hr_list_insert_between(list, position->previous, position, node);
}

hr_status_t hr_list_remove(hr_list_node_t *node)
{
    hr_list_t *list;

    if (node == NULL)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    list = node->list;
    if ((list == NULL) || (node == &list->root) || (list->size == 0U))
    {
        return HR_ERROR_INVALID_STATE;
    }

    node->previous->next = node->next;
    node->next->previous = node->previous;
    list->size--;

    node->previous = NULL;
    node->next = NULL;
    node->list = NULL;

    return HR_OK;
}

hr_list_node_t *hr_list_pop_front(hr_list_t *list)
{
    hr_list_node_t *node = hr_list_front(list);

    if ((node != NULL) && (hr_list_remove(node) != HR_OK))
    {
        return NULL;
    }

    return node;
}

hr_list_node_t *hr_list_pop_back(hr_list_t *list)
{
    hr_list_node_t *node = hr_list_back(list);

    if ((node != NULL) && (hr_list_remove(node) != HR_OK))
    {
        return NULL;
    }

    return node;
}

bool hr_list_validate(const hr_list_t *list)
{
    const hr_list_node_t *node;
    const hr_list_node_t *previous;
    size_t count = 0U;

    if ((list == NULL) || (list->root.list != list) ||
        (list->root.next == NULL) || (list->root.previous == NULL))
    {
        return false;
    }

    previous = &list->root;
    node = list->root.next;

    while (node != &list->root)
    {
        if ((node == NULL) || (node->list != list) ||
            (node->previous != previous) || (node->next == NULL))
        {
            return false;
        }

        previous = node;
        node = node->next;
        count++;

        if (count > list->size)
        {
            return false;
        }
    }

    return (count == list->size) &&
           (list->root.previous == previous) &&
           (list->root.previous->next == &list->root);
}
