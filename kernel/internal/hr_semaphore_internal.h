#ifndef HR_SEMAPHORE_INTERNAL_H
#define HR_SEMAPHORE_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

#include "hairtos/hr_semaphore.h"
#include "hr_wait_internal.h"

typedef struct
{
    uint32_t count;
    uint32_t max_count;
    hr_wait_list_t waiters;
    uint32_t magic;
} hr_semaphore_control_block_t;

_Static_assert(sizeof(hr_semaphore_control_block_t) <= sizeof(hr_semaphore_t),
               "HR_CFG_SEMAPHORE_STORAGE_BYTES is too small");

hr_semaphore_control_block_t *hr_semaphore_control_block(hr_semaphore_t *semaphore);
const hr_semaphore_control_block_t *hr_semaphore_control_block_const(
    const hr_semaphore_t *semaphore);
bool hr_semaphore_validate_internal(const hr_semaphore_t *semaphore);

#endif /* HR_SEMAPHORE_INTERNAL_H */
