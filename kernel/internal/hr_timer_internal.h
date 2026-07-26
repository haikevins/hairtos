#ifndef HR_TIMER_INTERNAL_H
#define HR_TIMER_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

#include "hairtos/hr_timer.h"
#include "hr_list_internal.h"
#include "hr_timeout_internal.h"

typedef struct
{
    hr_timeout_node_t timeout_node;
    hr_list_node_t pending_node;
    const char *name;
    hr_timer_callback_t callback;
    void *argument;
    hr_tick_t period_ticks;
    uint32_t pending_count;
    uint32_t magic;
    bool auto_reload;
    bool active;
    bool callback_running;
} hr_timer_control_block_t;

_Static_assert(sizeof(hr_timer_control_block_t) <= sizeof(hr_timer_t),
               "HR_CFG_TIMER_STORAGE_BYTES is too small");

hr_timer_control_block_t *hr_timer_control_block(hr_timer_t *timer);
const hr_timer_control_block_t *hr_timer_control_block_const(
    const hr_timer_t *timer);

void hr_timer_system_reset(void);
hr_status_t hr_timer_system_ensure_initialized(void);
void hr_timer_tick_from_isr(hr_tick_t now, bool *switch_required);
bool hr_timer_process_one_pending(void);
bool hr_timer_validate_internal(const hr_timer_t *timer);

#endif /* HR_TIMER_INTERNAL_H */
