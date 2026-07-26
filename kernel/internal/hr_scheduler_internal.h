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

/* Ready-set data-structure API. */
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

/* Fixed-priority scheduler policy. */
typedef struct
{
    hr_ready_set_t ready;
} hr_scheduler_t;

void hr_scheduler_init(hr_scheduler_t *scheduler);
hr_status_t hr_scheduler_add_ready(hr_scheduler_t *scheduler, hr_ready_node_t *node);
hr_status_t hr_scheduler_remove_ready(hr_scheduler_t *scheduler, hr_ready_node_t *node);
hr_ready_node_t *hr_scheduler_select_highest(const hr_scheduler_t *scheduler);
hr_status_t hr_scheduler_yield_current(hr_scheduler_t *scheduler,
                                       hr_ready_node_t *current);
bool hr_scheduler_should_preempt(const hr_scheduler_t *scheduler,
                                 const hr_ready_node_t *current);
bool hr_scheduler_has_equal_priority_peer(const hr_scheduler_t *scheduler,
                                          const hr_ready_node_t *current);
size_t hr_scheduler_ready_count(const hr_scheduler_t *scheduler);
uint32_t hr_scheduler_ready_bitmap(const hr_scheduler_t *scheduler);
bool hr_scheduler_validate(const hr_scheduler_t *scheduler);

#endif /* HR_SCHEDULER_INTERNAL_H */
