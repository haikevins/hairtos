#ifndef HR_QUEUE_INTERNAL_H
#define HR_QUEUE_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hairtos/hr_queue.h"
#include "hr_wait_internal.h"

typedef struct
{
    unsigned char *storage;
    size_t item_size;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    hr_wait_list_t send_waiters;
    hr_wait_list_t receive_waiters;
    uint32_t magic;
} hr_queue_control_block_t;

_Static_assert(sizeof(hr_queue_control_block_t) <= sizeof(hr_queue_t),
               "HR_CFG_QUEUE_STORAGE_BYTES is too small for the queue control block");

hr_queue_control_block_t *hr_queue_control_block(hr_queue_t *queue);
const hr_queue_control_block_t *hr_queue_control_block_const(const hr_queue_t *queue);
bool hr_queue_validate_internal(const hr_queue_t *queue);

#endif /* HR_QUEUE_INTERNAL_H */
