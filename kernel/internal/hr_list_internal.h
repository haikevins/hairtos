#ifndef HR_LIST_INTERNAL_H
#define HR_LIST_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>

#include "hairtos/hr_status.h"

typedef struct hr_list hr_list_t;

typedef struct hr_list_node
{
    struct hr_list_node *previous;
    struct hr_list_node *next;
    hr_list_t *list;
    void *owner;
} hr_list_node_t;

struct hr_list
{
    hr_list_node_t root;
    size_t size;
};

void hr_list_init(hr_list_t *list);
void hr_list_node_init(hr_list_node_t *node, void *owner);

bool hr_list_is_empty(const hr_list_t *list);
size_t hr_list_size(const hr_list_t *list);
bool hr_list_node_is_linked(const hr_list_node_t *node);
void *hr_list_node_owner(const hr_list_node_t *node);

hr_list_node_t *hr_list_front(const hr_list_t *list);
hr_list_node_t *hr_list_back(const hr_list_t *list);
hr_list_node_t *hr_list_next(const hr_list_t *list, const hr_list_node_t *node);
hr_list_node_t *hr_list_previous(const hr_list_t *list, const hr_list_node_t *node);

hr_status_t hr_list_push_front(hr_list_t *list, hr_list_node_t *node);
hr_status_t hr_list_push_back(hr_list_t *list, hr_list_node_t *node);
hr_status_t hr_list_insert_before(hr_list_t *list,
                                  hr_list_node_t *position,
                                  hr_list_node_t *node);
hr_status_t hr_list_remove(hr_list_node_t *node);
hr_list_node_t *hr_list_pop_front(hr_list_t *list);
hr_list_node_t *hr_list_pop_back(hr_list_t *list);

bool hr_list_validate(const hr_list_t *list);

#endif /* HR_LIST_INTERNAL_H */
