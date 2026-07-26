#ifndef HR_TASK_INTERNAL_H
#define HR_TASK_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hairtos/hr_task.h"
#include "hr_list_internal.h"
#include "hr_scheduler_internal.h"
#include "hr_timeout_internal.h"
#include "hr_wait_internal.h"

typedef enum
{
    HR_TASK_WAIT_NONE = 0,
    HR_TASK_WAIT_DELAY,
    HR_TASK_WAIT_QUEUE_SEND,
    HR_TASK_WAIT_QUEUE_RECEIVE,
    HR_TASK_WAIT_SEMAPHORE_TAKE,
    HR_TASK_WAIT_MUTEX_LOCK
} hr_task_wait_kind_t;

struct hr_task_control_block;
typedef void (*hr_task_wait_cleanup_t)(struct hr_task_control_block *task,
                                       hr_status_t result);

typedef struct hr_task_control_block
{
    hr_stack_t *stack_pointer;
    hr_stack_t *stack_low;
    hr_stack_t *stack_high;

    const char *name;
    hr_task_entry_t entry;
    void *argument;

    hr_task_state_t state;
    hr_task_state_t suspended_resume_state;
    hr_priority_t base_priority;
    hr_priority_t effective_priority;
    uint16_t reserved;

    hr_tick_t wake_tick;
    hr_tick_t time_slice_remaining;

    hr_ready_node_t ready_node;
    hr_wait_node_t wait_node;
    hr_timeout_node_t timeout_node;
    hr_list_node_t all_task_node;

    void *waiting_object;
    hr_wait_list_t *blocked_wait_list;
    void *wait_buffer;
    hr_task_wait_cleanup_t wait_cleanup;
    hr_status_t wait_result;
    hr_task_wait_kind_t wait_kind;

    hr_list_t owned_mutexes;
    size_t owned_mutex_count;
    size_t stack_words;
    uint32_t critical_nesting;
    uint32_t runtime_counter;
    uint32_t magic;
} hr_task_control_block_t;


_Static_assert(offsetof(hr_task_control_block_t, stack_pointer) == 0U,
               "The Cortex-M3 SVC handler requires stack_pointer at TCB offset zero");

_Static_assert(sizeof(hr_task_control_block_t) <= sizeof(hr_task_t),
               "HR_CFG_TASK_STORAGE_BYTES is too small for the internal TCB");

hr_task_control_block_t *hr_task_control_block(hr_task_t *task);
const hr_task_control_block_t *hr_task_control_block_const(const hr_task_t *task);
hr_status_t hr_task_transition_state(hr_task_t *task,
                                          hr_task_state_t expected,
                                          hr_task_state_t next);
void hr_task_exit_error(void);

#endif /* HR_TASK_INTERNAL_H */
