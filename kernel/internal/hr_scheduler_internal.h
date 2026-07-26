#ifndef HR_SCHEDULER_INTERNAL_H
#define HR_SCHEDULER_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hairtos_config.h"
#include "hairtos/hr_status.h"
#include "hairtos/hr_types.h"
#include "hr_list_internal.h"

typedef struct
{
    hr_list_node_t node;
    hr_priority_t priority;
} hr_ready_node_t;

typedef struct
{
    hr_list_t queues[HR_CFG_PRIORITY_COUNT];
    uint32_t bitmap;
    size_t count;
} hr_ready_set_t;

void hr_ready_node_init(hr_ready_node_t *node, void *owner, hr_priority_t priority);
void hr_ready_set_init(hr_ready_set_t *set);

bool hr_ready_set_is_empty(const hr_ready_set_t *set);
size_t hr_ready_set_size(const hr_ready_set_t *set);
uint32_t hr_ready_set_bitmap(const hr_ready_set_t *set);

hr_status_t hr_ready_set_insert(hr_ready_set_t *set, hr_ready_node_t *node);
hr_status_t hr_ready_set_remove(hr_ready_set_t *set, hr_ready_node_t *node);
hr_ready_node_t *hr_ready_set_peek_highest(const hr_ready_set_t *set);
hr_status_t hr_ready_set_rotate_highest(hr_ready_set_t *set);

bool hr_ready_set_validate(const hr_ready_set_t *set);

#endif /* HR_SCHEDULER_INTERNAL_H */
