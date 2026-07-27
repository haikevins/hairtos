
#include "hairtos/hr_time.h"
#include "hr_kernel_internal.h"

hr_tick_t hr_time_now(void)
{
    return hr_kernel_get_tick();
}
