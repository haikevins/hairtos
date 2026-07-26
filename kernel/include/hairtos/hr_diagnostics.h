#ifndef HR_DIAGNOSTICS_H
#define HR_DIAGNOSTICS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hr_status.h"
#include "hr_task.h"
#include "hr_types.h"

#define HR_DIAGNOSTICS_TASK_NAME_LENGTH 24U
#define HR_DIAGNOSTICS_RECORD_VERSION   1U

typedef enum
{
    HR_PANIC_NONE = 0,
    HR_PANIC_ASSERT,
    HR_PANIC_KERNEL_INVARIANT,
    HR_PANIC_STACK_OVERFLOW,
    HR_PANIC_NMI,
    HR_PANIC_HARDFAULT,
    HR_PANIC_MEMMANAGE,
    HR_PANIC_BUSFAULT,
    HR_PANIC_USAGEFAULT,
    HR_PANIC_USER
} hr_panic_reason_t;

typedef struct
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;
    uint32_t exception_return;
    uint32_t exception_number;
    uint32_t cfsr;
    uint32_t hfsr;
    uint32_t dfsr;
    uint32_t afsr;
    uint32_t mmfar;
    uint32_t bfar;
    uint32_t shcsr;
} hr_fault_context_t;

typedef struct
{
    uint32_t signature;
    uint32_t version;
    uint32_t boot_count;
    uint32_t sequence;
    hr_panic_reason_t reason;
    hr_tick_t tick;
    uintptr_t task_address;
    char task_name[HR_DIAGNOSTICS_TASK_NAME_LENGTH];
    uint32_t source_line;
    uint32_t source_hash;
    hr_fault_context_t fault;
} hr_panic_record_t;

typedef struct
{
    uint32_t systick_count;
    uint32_t pendsv_count;
    uint32_t task_switch_count;
    uint32_t yield_count;
    uint32_t block_count;
    uint32_t preemption_count;
    uint32_t time_slice_count;
    uint32_t timeout_wakeup_count;
    uint32_t invariant_check_count;
    uint32_t invariant_failure_count;
    uint32_t stack_check_count;
    uint32_t stack_failure_count;
    uint32_t panic_count;
} hr_runtime_statistics_t;

typedef struct
{
    const hr_task_t *task;
    const char *name;
    hr_task_state_t state;
    hr_priority_t base_priority;
    hr_priority_t effective_priority;
    size_t stack_words;
    size_t stack_free_words;
    size_t stack_used_words;
    uint32_t runtime_ticks;
    bool stack_guard_valid;
} hr_task_diagnostics_t;

typedef struct
{
    bool kernel_invariants_valid;
    bool all_stack_guards_valid;
    size_t task_count;
    size_t ready_task_count;
    size_t timeout_task_count;
    uint32_t ready_bitmap;
    size_t minimum_stack_free_words;
    size_t low_stack_task_count;
    const hr_task_t *current_task;
    const char *current_task_name;
} hr_health_report_t;

void hr_diagnostics_initialize(void);
void hr_diagnostics_reset_runtime_statistics(void);
void hr_diagnostics_get_runtime_statistics(hr_runtime_statistics_t *statistics);

hr_status_t hr_diagnostics_get_task(const hr_task_t *task,
                                    hr_task_diagnostics_t *diagnostics);
hr_status_t hr_diagnostics_run_health_check(hr_health_report_t *report);

bool hr_diagnostics_get_last_panic(hr_panic_record_t *record);
void hr_diagnostics_clear_last_panic(void);
void hr_diagnostics_record_panic(hr_panic_reason_t reason,
                                 const char *source_file,
                                 uint32_t source_line);
void hr_diagnostics_record_fault(hr_panic_reason_t reason,
                                 const hr_fault_context_t *fault);
const char *hr_diagnostics_panic_reason_string(hr_panic_reason_t reason);

#endif /* HR_DIAGNOSTICS_H */
