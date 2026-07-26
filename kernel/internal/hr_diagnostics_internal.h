#ifndef HR_DIAGNOSTICS_INTERNAL_H
#define HR_DIAGNOSTICS_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

#include "hairtos_config.h"
#include "hairtos/hr_diagnostics.h"

#if (HR_CFG_ENABLE_DIAGNOSTICS == 1)
_Noreturn void hr_diagnostics_platform_halt(void);
void hr_diagnostics_internal_note_tick(void);
void hr_diagnostics_internal_note_switch(uint32_t switch_reasons,
                                         bool task_changed);
void hr_diagnostics_internal_note_timeout_wakeup(void);
void hr_diagnostics_internal_note_invariant_result(bool valid);
void hr_diagnostics_internal_note_stack_result(bool valid);
void hr_diagnostics_internal_note_kernel_panic(uint32_t source_line);
#else
static inline void hr_diagnostics_internal_note_tick(void) {}
static inline void hr_diagnostics_internal_note_switch(uint32_t switch_reasons,
                                                       bool task_changed)
{
    (void)switch_reasons;
    (void)task_changed;
}
static inline void hr_diagnostics_internal_note_timeout_wakeup(void) {}
static inline void hr_diagnostics_internal_note_invariant_result(bool valid)
{
    (void)valid;
}
static inline void hr_diagnostics_internal_note_stack_result(bool valid)
{
    (void)valid;
}
static inline void hr_diagnostics_internal_note_kernel_panic(uint32_t source_line)
{
    (void)source_line;
}
#endif

#endif /* HR_DIAGNOSTICS_INTERNAL_H */
