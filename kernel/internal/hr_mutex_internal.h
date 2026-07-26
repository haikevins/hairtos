#ifndef HR_MUTEX_INTERNAL_H
#define HR_MUTEX_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

#include "hairtos/hr_mutex.h"
#include "hr_list_internal.h"
#include "hr_wait_internal.h"

struct hr_task_control_block;

typedef struct
{
    hr_task_t *owner;
    uint32_t recursion_count;
    bool recursive;
    uint8_t reserved[3];
    hr_wait_list_t waiters;
    hr_list_node_t owner_node;
    uint32_t magic;
} hr_mutex_control_block_t;

_Static_assert(sizeof(hr_mutex_control_block_t) <= sizeof(hr_mutex_t),
               "HR_CFG_MUTEX_STORAGE_BYTES is too small");

hr_mutex_control_block_t *hr_mutex_control_block(hr_mutex_t *mutex);
const hr_mutex_control_block_t *hr_mutex_control_block_const(const hr_mutex_t *mutex);
bool hr_mutex_validate_internal(const hr_mutex_t *mutex);
void hr_mutex_wait_cleanup(struct hr_task_control_block *task,
                           hr_status_t result);
hr_status_t hr_mutex_recompute_owner_priority(hr_mutex_control_block_t *mutex);

#endif /* HR_MUTEX_INTERNAL_H */
