#ifndef HR_KERNEL_H
#define HR_KERNEL_H

#include <stdbool.h>
#include <stddef.h>

#include "hr_status.h"
#include "hr_types.h"

typedef enum
{
    HR_KERNEL_STATE_RESET = 0,
    HR_KERNEL_STATE_INITIALIZED,
    HR_KERNEL_STATE_RUNNING,
    HR_KERNEL_STATE_PANIC
} hr_kernel_state_t;

hr_status_t hr_kernel_init(void);
hr_status_t hr_kernel_start(void);

bool hr_kernel_is_running(void);
hr_kernel_state_t hr_kernel_get_state(void);
hr_tick_t hr_kernel_get_tick(void);
size_t hr_kernel_get_task_count(void);

#endif /* HR_KERNEL_H */
