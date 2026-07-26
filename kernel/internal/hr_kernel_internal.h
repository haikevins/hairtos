#ifndef HR_KERNEL_INTERNAL_H
#define HR_KERNEL_INTERNAL_H

#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hr_task_internal.h"


typedef struct
{
    size_t task_count;
    size_t ready_task_count;
    size_t timeout_task_count;
    uint32_t ready_bitmap;
    hr_task_t *current_task;
} hr_kernel_internal_snapshot_t;

/* Read directly by Cortex-M handlers. The saved SP is TCB field zero. */
extern hr_task_control_block_t *g_hr_current_task_control_block;

hr_status_t hr_kernel_register_task(hr_task_t *task);
hr_status_t hr_kernel_prepare_start(void);
hr_status_t hr_kernel_delay_current(hr_tick_t delay_ticks);
hr_status_t hr_kernel_suspend_task(hr_task_t *task, bool *switch_required);
hr_status_t hr_kernel_resume_task(hr_task_t *task, bool *switch_required);
hr_status_t hr_kernel_block_current_on_wait_list(hr_wait_list_t *wait_list,
                                                 void *object,
                                                 hr_task_wait_kind_t wait_kind,
                                                 void *buffer,
                                                 hr_tick_t timeout);
hr_status_t hr_kernel_block_current_on_wait_list_ex(
    hr_wait_list_t *wait_list,
    void *object,
    hr_task_wait_kind_t wait_kind,
    void *buffer,
    hr_tick_t timeout,
    hr_task_wait_cleanup_t cleanup);
hr_status_t hr_kernel_unblock_waiter(hr_wait_node_t *wait_node,
                                     hr_status_t result,
                                     bool *higher_priority_task_woken);
void hr_kernel_request_yield(void);
void hr_kernel_tick_from_isr(void);
void hr_kernel_select_next_from_pendsv(void);
hr_task_t *hr_kernel_current_task_internal(void);
hr_status_t hr_kernel_set_task_effective_priority(
    hr_task_control_block_t *control_block,
    hr_priority_t priority);
bool hr_kernel_current_should_preempt(void);
bool hr_kernel_validate_internal(void);
hr_status_t hr_kernel_get_internal_snapshot(hr_kernel_internal_snapshot_t *snapshot);
hr_task_t *hr_kernel_get_task_by_index_internal(size_t index);

#endif /* HR_KERNEL_INTERNAL_H */
