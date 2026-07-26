#include <stddef.h>
#include <stdint.h>

#include "hairtos_config.h"
#include "hairtos/hr_context.h"
#include "hairtos/hr_diagnostics.h"
#include "hairtos/hr_hooks.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hr_diagnostics_internal.h"
#include "hr_kernel_internal.h"
#include "hr_task_internal.h"

#define HR_DIAGNOSTICS_SWITCH_YIELD      (UINT32_C(1) << 0U)
#define HR_DIAGNOSTICS_SWITCH_BLOCK      (UINT32_C(1) << 1U)
#define HR_DIAGNOSTICS_SWITCH_PREEMPT    (UINT32_C(1) << 2U)
#define HR_DIAGNOSTICS_SWITCH_TIME_SLICE (UINT32_C(1) << 3U)

#if defined(__GNUC__)
#define HR_WEAK __attribute__((weak))
#define HR_NOINIT __attribute__((section(".noinit.hairtos"), used))
#else
#define HR_WEAK
#define HR_NOINIT
#endif

static HR_NOINIT hr_panic_record_t g_panic_record;
static hr_runtime_statistics_t g_runtime_statistics;
static bool g_diagnostics_initialized;

static void hr_diagnostics_zero(void *memory, size_t bytes)
{
    unsigned char *destination = (unsigned char *)memory;
    size_t index;

    for (index = 0U; index < bytes; index++)
    {
        destination[index] = 0U;
    }
}


static void hr_diagnostics_copy(void *destination,
                                const void *source,
                                size_t bytes)
{
    unsigned char *output = (unsigned char *)destination;
    const unsigned char *input = (const unsigned char *)source;
    size_t index;

    for (index = 0U; index < bytes; index++)
    {
        output[index] = input[index];
    }
}

static uint32_t hr_diagnostics_hash_string(const char *text)
{
    uint32_t hash = UINT32_C(2166136261);

    if (text == NULL)
    {
        return 0U;
    }

    while (*text != '\0')
    {
        hash ^= (uint32_t)(unsigned char)*text;
        hash *= UINT32_C(16777619);
        text++;
    }

    return hash;
}

static void hr_diagnostics_copy_name(char *destination,
                                     size_t capacity,
                                     const char *source)
{
    size_t index = 0U;

    if ((destination == NULL) || (capacity == 0U))
    {
        return;
    }

    if (source != NULL)
    {
        while ((index + 1U < capacity) && (source[index] != '\0'))
        {
            destination[index] = source[index];
            index++;
        }
    }

    destination[index] = '\0';
    index++;
    while (index < capacity)
    {
        destination[index] = '\0';
        index++;
    }
}

static bool hr_diagnostics_record_header_valid(void)
{
    return (g_panic_record.signature == HR_CFG_DIAGNOSTICS_SIGNATURE) &&
           (g_panic_record.version == HR_DIAGNOSTICS_RECORD_VERSION);
}

void hr_diagnostics_initialize(void)
{
    hr_irq_state_t irq_state;

    if (g_diagnostics_initialized)
    {
        return;
    }

    irq_state = hr_critical_enter();
    if (!hr_diagnostics_record_header_valid())
    {
        hr_diagnostics_zero(&g_panic_record, sizeof(g_panic_record));
        g_panic_record.signature = HR_CFG_DIAGNOSTICS_SIGNATURE;
        g_panic_record.version = HR_DIAGNOSTICS_RECORD_VERSION;
    }

    g_panic_record.boot_count++;
    hr_diagnostics_zero(&g_runtime_statistics, sizeof(g_runtime_statistics));
    g_diagnostics_initialized = true;
    hr_critical_exit(irq_state);
}

void hr_diagnostics_reset_runtime_statistics(void)
{
    hr_irq_state_t irq_state;

    hr_diagnostics_initialize();
    irq_state = hr_critical_enter();
    hr_diagnostics_zero(&g_runtime_statistics, sizeof(g_runtime_statistics));
    hr_critical_exit(irq_state);
}

void hr_diagnostics_get_runtime_statistics(hr_runtime_statistics_t *statistics)
{
    hr_irq_state_t irq_state;

    if (statistics == NULL)
    {
        return;
    }

    hr_diagnostics_initialize();
    irq_state = hr_critical_enter();
    hr_diagnostics_copy(statistics,
                        &g_runtime_statistics,
                        sizeof(*statistics));
    hr_critical_exit(irq_state);
}

hr_status_t hr_diagnostics_get_task(const hr_task_t *task,
                                    hr_task_diagnostics_t *diagnostics)
{
    const hr_task_control_block_t *control_block;
    size_t free_words;

    if ((task == NULL) || (diagnostics == NULL) || !hr_task_is_valid(task))
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    control_block = hr_task_control_block_const(task);
    free_words = hr_task_get_stack_high_watermark(task);

    diagnostics->task = task;
    diagnostics->name = control_block->name;
    diagnostics->state = control_block->state;
    diagnostics->base_priority = control_block->base_priority;
    diagnostics->effective_priority = control_block->effective_priority;
    diagnostics->stack_words = control_block->stack_words;
    diagnostics->stack_free_words = free_words;
    diagnostics->stack_used_words =
        (free_words <= control_block->stack_words) ?
        (control_block->stack_words - free_words) : control_block->stack_words;
    diagnostics->runtime_ticks = control_block->runtime_counter;
    diagnostics->stack_guard_valid = hr_task_stack_guard_is_valid(task);
    return HR_OK;
}

hr_status_t hr_diagnostics_run_health_check(hr_health_report_t *report)
{
    hr_kernel_internal_snapshot_t snapshot;
    hr_irq_state_t irq_state;
    size_t index;
    size_t minimum_free = SIZE_MAX;
    size_t low_stack_count = 0U;
    bool stacks_valid = true;
    bool kernel_valid;

    if (report == NULL)
    {
        return HR_ERROR_INVALID_ARGUMENT;
    }

    hr_diagnostics_initialize();
    irq_state = hr_critical_enter();

    if (hr_kernel_get_internal_snapshot(&snapshot) != HR_OK)
    {
        hr_critical_exit(irq_state);
        return HR_ERROR_INVALID_STATE;
    }

    kernel_valid = hr_kernel_validate_internal();
    hr_diagnostics_internal_note_invariant_result(kernel_valid);

    for (index = 0U; index < snapshot.task_count; index++)
    {
        hr_task_t *task = hr_kernel_get_task_by_index_internal(index);
        hr_task_diagnostics_t task_diagnostics;

        if ((task == NULL) ||
            (hr_diagnostics_get_task(task, &task_diagnostics) != HR_OK))
        {
            kernel_valid = false;
            continue;
        }

        hr_diagnostics_internal_note_stack_result(
            task_diagnostics.stack_guard_valid);
        if (!task_diagnostics.stack_guard_valid)
        {
            stacks_valid = false;
            hr_hook_stack_overflow(task, task_diagnostics.name);
        }

        if (task_diagnostics.stack_free_words < minimum_free)
        {
            minimum_free = task_diagnostics.stack_free_words;
        }
        if (task_diagnostics.stack_free_words <
            (size_t)HR_CFG_DIAGNOSTICS_STACK_MARGIN_WORDS)
        {
            low_stack_count++;
        }
    }

    report->kernel_invariants_valid = kernel_valid;
    report->all_stack_guards_valid = stacks_valid;
    report->task_count = snapshot.task_count;
    report->ready_task_count = snapshot.ready_task_count;
    report->timeout_task_count = snapshot.timeout_task_count;
    report->ready_bitmap = snapshot.ready_bitmap;
    report->minimum_stack_free_words =
        (minimum_free == SIZE_MAX) ? 0U : minimum_free;
    report->low_stack_task_count = low_stack_count;
    report->current_task = snapshot.current_task;
    report->current_task_name =
        (snapshot.current_task != NULL) ?
        hr_task_get_name(snapshot.current_task) : NULL;

    hr_critical_exit(irq_state);
    return (kernel_valid && stacks_valid) ? HR_OK : HR_ERROR_INTERNAL;
}

bool hr_diagnostics_get_last_panic(hr_panic_record_t *record)
{
    hr_irq_state_t irq_state;
    bool available;

    if (record == NULL)
    {
        return false;
    }

    hr_diagnostics_initialize();
    irq_state = hr_critical_enter();
    available = hr_diagnostics_record_header_valid() &&
                (g_panic_record.reason != HR_PANIC_NONE);
    if (available)
    {
        hr_diagnostics_copy(record, &g_panic_record, sizeof(*record));
    }
    hr_critical_exit(irq_state);
    return available;
}

void hr_diagnostics_clear_last_panic(void)
{
    hr_irq_state_t irq_state;
    uint32_t boot_count;
    uint32_t sequence;

    hr_diagnostics_initialize();
    irq_state = hr_critical_enter();
    boot_count = g_panic_record.boot_count;
    sequence = g_panic_record.sequence;
    hr_diagnostics_zero(&g_panic_record, sizeof(g_panic_record));
    g_panic_record.signature = HR_CFG_DIAGNOSTICS_SIGNATURE;
    g_panic_record.version = HR_DIAGNOSTICS_RECORD_VERSION;
    g_panic_record.boot_count = boot_count;
    g_panic_record.sequence = sequence;
    hr_critical_exit(irq_state);
}

static void hr_diagnostics_prepare_record(hr_panic_reason_t reason)
{
    hr_task_t *current_task = hr_task_current();

    g_panic_record.signature = HR_CFG_DIAGNOSTICS_SIGNATURE;
    g_panic_record.version = HR_DIAGNOSTICS_RECORD_VERSION;
    g_panic_record.sequence++;
    g_panic_record.reason = reason;
    g_panic_record.tick = hr_kernel_get_tick();
    g_panic_record.task_address = (uintptr_t)current_task;
    hr_diagnostics_copy_name(
        g_panic_record.task_name,
        sizeof(g_panic_record.task_name),
        ((current_task != NULL) && hr_task_is_valid(current_task)) ?
        hr_task_get_name(current_task) : NULL);
    g_runtime_statistics.panic_count++;
}

void hr_diagnostics_record_panic(hr_panic_reason_t reason,
                                 const char *source_file,
                                 uint32_t source_line)
{
    hr_irq_state_t irq_state;
    hr_panic_record_t record;

    hr_diagnostics_initialize();
    irq_state = hr_critical_enter();
    hr_diagnostics_prepare_record(reason);
    g_panic_record.source_line = source_line;
    g_panic_record.source_hash = hr_diagnostics_hash_string(source_file);
    hr_diagnostics_zero(&g_panic_record.fault, sizeof(g_panic_record.fault));
    hr_diagnostics_copy(&record, &g_panic_record, sizeof(record));
    hr_critical_exit(irq_state);
    hr_hook_panic(&record);
}

void hr_diagnostics_record_fault(hr_panic_reason_t reason,
                                 const hr_fault_context_t *fault)
{
    hr_irq_state_t irq_state;
    hr_panic_record_t record;

    hr_diagnostics_initialize();
    irq_state = hr_critical_enter();
    hr_diagnostics_prepare_record(reason);
    g_panic_record.source_line = 0U;
    g_panic_record.source_hash = 0U;
    if (fault != NULL)
    {
        hr_diagnostics_copy(&g_panic_record.fault,
                            fault,
                            sizeof(g_panic_record.fault));
    }
    else
    {
        hr_diagnostics_zero(&g_panic_record.fault,
                            sizeof(g_panic_record.fault));
    }
    hr_diagnostics_copy(&record, &g_panic_record, sizeof(record));
    hr_critical_exit(irq_state);
    hr_hook_panic(&record);
}

const char *hr_diagnostics_panic_reason_string(hr_panic_reason_t reason)
{
    switch (reason)
    {
        case HR_PANIC_NONE: return "none";
        case HR_PANIC_ASSERT: return "assert";
        case HR_PANIC_KERNEL_INVARIANT: return "kernel-invariant";
        case HR_PANIC_STACK_OVERFLOW: return "stack-overflow";
        case HR_PANIC_NMI: return "nmi";
        case HR_PANIC_HARDFAULT: return "hardfault";
        case HR_PANIC_MEMMANAGE: return "memmanage";
        case HR_PANIC_BUSFAULT: return "busfault";
        case HR_PANIC_USAGEFAULT: return "usagefault";
        case HR_PANIC_USER: return "user";
        default: return "unknown";
    }
}

void hr_diagnostics_internal_note_tick(void)
{
    g_runtime_statistics.systick_count++;
}

void hr_diagnostics_internal_note_switch(uint32_t switch_reasons,
                                         bool task_changed)
{
    g_runtime_statistics.pendsv_count++;
    if (task_changed)
    {
        g_runtime_statistics.task_switch_count++;
    }
    if ((switch_reasons & HR_DIAGNOSTICS_SWITCH_YIELD) != 0U)
    {
        g_runtime_statistics.yield_count++;
    }
    if ((switch_reasons & HR_DIAGNOSTICS_SWITCH_BLOCK) != 0U)
    {
        g_runtime_statistics.block_count++;
    }
    if ((switch_reasons & HR_DIAGNOSTICS_SWITCH_PREEMPT) != 0U)
    {
        g_runtime_statistics.preemption_count++;
    }
    if ((switch_reasons & HR_DIAGNOSTICS_SWITCH_TIME_SLICE) != 0U)
    {
        g_runtime_statistics.time_slice_count++;
    }
}

void hr_diagnostics_internal_note_timeout_wakeup(void)
{
    g_runtime_statistics.timeout_wakeup_count++;
}

void hr_diagnostics_internal_note_invariant_result(bool valid)
{
    g_runtime_statistics.invariant_check_count++;
    if (!valid)
    {
        g_runtime_statistics.invariant_failure_count++;
    }
}

void hr_diagnostics_internal_note_stack_result(bool valid)
{
    g_runtime_statistics.stack_check_count++;
    if (!valid)
    {
        g_runtime_statistics.stack_failure_count++;
    }
}

void hr_diagnostics_internal_note_kernel_panic(uint32_t source_line)
{
    hr_diagnostics_record_panic(HR_PANIC_KERNEL_INVARIANT,
                                "kernel/src/hr_kernel.c",
                                source_line);
}

HR_WEAK void hr_hook_panic(const hr_panic_record_t *record)
{
    (void)record;
}

HR_WEAK void hr_hook_stack_overflow(const hr_task_t *task,
                                    const char *task_name)
{
    (void)task;
    (void)task_name;
}

HR_WEAK void hr_hook_assert_failed(const char *expression,
                                   const char *source_file,
                                   uint32_t source_line)
{
    (void)expression;
    (void)source_file;
    (void)source_line;
}

HR_WEAK _Noreturn void hr_diagnostics_platform_halt(void)
{
    for (;;)
    {
        /* Target ports may override this weak fail-stop implementation. */
    }
}

_Noreturn void hr_assert_failed(const char *expression,
                                const char *source_file,
                                uint32_t source_line)
{
    hr_hook_assert_failed(expression, source_file, source_line);
    hr_diagnostics_record_panic(HR_PANIC_ASSERT, source_file, source_line);
    hr_diagnostics_platform_halt();
}
