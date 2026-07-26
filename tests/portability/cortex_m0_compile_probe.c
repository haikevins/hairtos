#include <stddef.h>
#include <stdint.h>

#include "hairtos/hr_status.h"
#include "hairtos/hr_types.h"
#include "hr_list_internal.h"
#include "hr_scheduler_internal.h"
#include "hr_timeout_internal.h"
#include "hr_wait_internal.h"

_Static_assert(sizeof(hr_tick_t) == sizeof(uint32_t),
               "hairtos tick must remain 32-bit");
_Static_assert(sizeof(hr_priority_t) == sizeof(uint8_t),
               "hairtos priority must remain 8-bit");
_Static_assert(HR_CFG_USE_FPU == 0,
               "Cortex-M0 proof assumes no FPU context");

void hairtos_cortex_m0_compile_probe(void)
{
    hr_list_t list;
    hr_scheduler_t scheduler;
    hr_timeout_list_t timeouts;
    hr_wait_list_t waiters;

    hr_list_init(&list);
    hr_scheduler_init(&scheduler);
    hr_timeout_list_init(&timeouts, 0U);
    hr_wait_list_init(&waiters);

    (void)hr_list_validate(&list);
    (void)hr_scheduler_validate(&scheduler);
    (void)hr_timeout_list_validate(&timeouts);
    (void)hr_wait_list_validate(&waiters);
}
