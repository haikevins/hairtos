#ifndef HR_KERNEL_H
#define HR_KERNEL_H
#include "hr_status.h"
#include "hr_types.h"
hr_status_t hr_kernel_init(void);
hr_status_t hr_kernel_start(void);
bool hr_kernel_is_running(void);
hr_tick_t hr_kernel_get_tick(void);
#endif
