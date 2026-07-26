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

typedef struct
{
    hr_stack_t *stack_pointer;
    hr_stack_t *stack_low;
    hr_stack_t *stack_high;

    const char *name;
    hr_task_entry_t entry;
    void *argument;

    hr_task_state_t state;
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
    size_t stack_words;
    uint32_t critical_nesting;
    uint32_t runtime_counter;
    uint32_t magic;
} hr_task_control_block_t;

_Static_assert(sizeof(hr_task_control_block_t) <= sizeof(hr_task_t),
               "HR_CFG_TASK_STORAGE_BYTES is too small for the internal TCB");

hr_task_control_block_t *hr_task_control_block(hr_task_t *task);
const hr_task_control_block_t *hr_task_control_block_const(const hr_task_t *task);
void hr_task_exit_error(void);

#endif /* HR_TASK_INTERNAL_H */
