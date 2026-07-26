#ifndef HR_WAIT_INTERNAL_H
#define HR_WAIT_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>

#include "hairtos/hr_status.h"
#include "hairtos/hr_types.h"
#include "hr_list_internal.h"

typedef struct
{
    hr_list_node_t node;
    hr_priority_t priority;
} hr_wait_node_t;

typedef struct
{
    hr_list_t list;
} hr_wait_list_t;

void hr_wait_node_init(hr_wait_node_t *node, void *owner, hr_priority_t priority);
void hr_wait_list_init(hr_wait_list_t *list);

bool hr_wait_list_is_empty(const hr_wait_list_t *list);
size_t hr_wait_list_size(const hr_wait_list_t *list);

hr_status_t hr_wait_list_insert(hr_wait_list_t *list, hr_wait_node_t *node);
hr_status_t hr_wait_list_remove(hr_wait_list_t *list, hr_wait_node_t *node);
hr_wait_node_t *hr_wait_list_peek(const hr_wait_list_t *list);
hr_wait_node_t *hr_wait_list_pop(hr_wait_list_t *list);

bool hr_wait_list_validate(const hr_wait_list_t *list);

#endif /* HR_WAIT_INTERNAL_H */
