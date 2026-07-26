#ifndef HR_TIMEOUT_INTERNAL_H
#define HR_TIMEOUT_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>

#include "hairtos_config.h"
#include "hairtos/hr_status.h"
#include "hairtos/hr_types.h"
#include "hr_list_internal.h"

typedef struct
{
    hr_list_node_t node;
    hr_tick_t wake_tick;
} hr_timeout_node_t;

typedef struct
{
    hr_list_t lists[2];
    hr_list_t *current;
    hr_list_t *overflow;
    hr_tick_t last_tick;
    size_t count;
} hr_timeout_list_t;

void hr_timeout_node_init(hr_timeout_node_t *node, void *owner);
void hr_timeout_list_init(hr_timeout_list_t *list, hr_tick_t start_tick);

bool hr_timeout_list_is_empty(const hr_timeout_list_t *list);
size_t hr_timeout_list_size(const hr_timeout_list_t *list);
hr_tick_t hr_timeout_list_last_tick(const hr_timeout_list_t *list);

hr_status_t hr_timeout_list_insert(hr_timeout_list_t *list,
                                   hr_timeout_node_t *node,
                                   hr_tick_t delay_ticks);
hr_status_t hr_timeout_list_remove(hr_timeout_list_t *list,
                                   hr_timeout_node_t *node);
hr_status_t hr_timeout_list_advance(hr_timeout_list_t *list,
                                    hr_tick_t now,
                                    hr_list_t *expired_nodes);

bool hr_timeout_list_validate(const hr_timeout_list_t *list);

#endif /* HR_TIMEOUT_INTERNAL_H */
