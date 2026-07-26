#ifndef HR_TYPES_H
#define HR_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hairtos_config.h"

typedef uint32_t hr_tick_t;
typedef uint32_t hr_stack_t;
typedef uint8_t hr_priority_t;
typedef uint32_t hr_irq_state_t;

typedef union hr_task
{
    max_align_t alignment;
    unsigned char storage[HR_CFG_TASK_STORAGE_BYTES];
} hr_task_t;

typedef union hr_queue
{
    max_align_t alignment;
    unsigned char storage[HR_CFG_QUEUE_STORAGE_BYTES];
} hr_queue_t;

typedef union hr_semaphore
{
    max_align_t alignment;
    unsigned char storage[HR_CFG_SEMAPHORE_STORAGE_BYTES];
} hr_semaphore_t;

typedef union hr_mutex
{
    max_align_t alignment;
    unsigned char storage[HR_CFG_MUTEX_STORAGE_BYTES];
} hr_mutex_t;
typedef union hr_timer
{
    max_align_t alignment;
    unsigned char storage[HR_CFG_TIMER_STORAGE_BYTES];
} hr_timer_t;

typedef void (*hr_task_entry_t)(void *argument);
typedef void (*hr_timer_callback_t)(void *argument);

#endif /* HR_TYPES_H */
