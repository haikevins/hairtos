#ifndef HR_KERNEL_INTERNAL_H
#define HR_KERNEL_INTERNAL_H

#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hr_task_internal.h"

/* Read directly by Cortex-M handlers. The saved SP is TCB field zero. */
extern hr_task_control_block_t *g_hr_current_task_control_block;

hr_status_t hr_kernel_register_task(hr_task_t *task);
hr_status_t hr_kernel_prepare_start(void);
hr_status_t hr_kernel_delay_current(hr_tick_t delay_ticks);
void hr_kernel_request_yield(void);
void hr_kernel_tick_from_isr(void);
void hr_kernel_select_next_from_pendsv(void);
hr_task_t *hr_kernel_current_task_internal(void);

#endif /* HR_KERNEL_INTERNAL_H */
