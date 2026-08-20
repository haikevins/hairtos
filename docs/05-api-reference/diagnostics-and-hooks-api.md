# Diagnostics and Hooks API

> **Scope:** Public API contracts for `hairtos 1.0.0-rc1`; internal helpers are not part of the compatibility surface.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [Next →](haievent-api.md)

## Table of Contents

- [API Principles](#principles)
- [Function surface](#functions)
- [Context / blocking contract](#context)
- [Ownership and Lifetime](#ownership)
- [Error semantics](#errors)
- [Source map](#source-map)
- [References](#references)

<a id="principles"></a>
## API Principles

v1 diagnostics combine runtime counters, task/stack snapshots, kernel invariant checks, and a retained panic/fault record in `.noinit`. When diagnostics are enabled, the port also enables Usage/Bus/Mem faults and traps unaligned access/divide-by-zero so failures become diagnostic data rather than silent corruption.

- Public handles are opaque storage; application code must not cast them to internal TCB/control-block types.
- Functions return `hr_status_t` when an operation can fail; boolean/size/metadata queries return neutral values for invalid objects according to the current implementation.
- Blocking APIs are for task context only while the kernel is RUNNING; ISR variants are named `_from_isr` and never block.
- The caller retains ownership of static backing storage; create/init does not copy the entire object into a kernel heap.

<a id="functions"></a>
## Function surface

### `kernel/include/hairtos/hr_diagnostics.h`

```c
void hr_diagnostics_initialize(void);
void hr_diagnostics_reset_runtime_statistics(void);
void hr_diagnostics_get_runtime_statistics(hr_runtime_statistics_t *statistics);
hr_status_t hr_diagnostics_get_task(const hr_task_t *task, hr_task_diagnostics_t *diagnostics);
hr_status_t hr_diagnostics_run_health_check(hr_health_report_t *report);
bool hr_diagnostics_get_last_panic(hr_panic_record_t *record);
void hr_diagnostics_clear_last_panic(void);
void hr_diagnostics_record_panic(hr_panic_reason_t reason, const char *source_file, uint32_t source_line);
void hr_diagnostics_record_fault(hr_panic_reason_t reason, const hr_fault_context_t *fault);
const char *hr_diagnostics_panic_reason_string(hr_panic_reason_t reason);
```

### `kernel/include/hairtos/hr_hooks.h`

```c
void hr_hook_panic(const hr_panic_record_t *record);
void hr_hook_stack_overflow(const hr_task_t *task, const char *task_name);
void hr_hook_assert_failed(const char *expression, const char *source_file, uint32_t source_line);
_Noreturn void hr_assert_failed(const char *expression, const char *source_file, uint32_t source_line);
hr_assert_failed(#expression, __FILE__, (uint32_t)__LINE__); \
```

<a id="context"></a>
## Context / blocking contract

| Group | Task context | ISR context | May block |
| --- | --- | --- | --- |
| Query/getter | Yes | Only when the implementation requires no blocking or long lock | No |
| Regular `send/take/lock/delay` APIs | Yes | No | Yes when timeout is not `HR_NO_WAIT` |
| `_from_isr` APIs | Not the primary use case | Yes | No |
| `hr_critical_enter/exit` | Yes | Yes, but must remain extremely short | No |

<a id="ownership"></a>
## Ownership and Lifetime

- `hr_task_t` + task stack: caller-owned for the entire task lifetime.
- Queue: caller-owned queue object + item storage.
- Semaphore/mutex/timer: caller-owned opaque object storage.
- haievent Active Object: caller-owned AO storage + stack + event-pointer queue; dynamic events have independent reference counts.
- Backing storage must not be moved, freed, or reused while the object remains valid/registered.

<a id="errors"></a>
## Error semantics

The currently defined public status values are:

```text
HR_OK
HR_ERROR_INVALID_ARGUMENT
HR_ERROR_INVALID_STATE
HR_ERROR_TIMEOUT
HR_ERROR_QUEUE_FULL / HR_ERROR_QUEUE_EMPTY
HR_ERROR_NO_MEMORY
HR_ERROR_NOT_OWNER
HR_ERROR_FROM_ISR
HR_ERROR_NOT_SUPPORTED
HR_ERROR_INTERNAL
HR_ERROR_SEMAPHORE_EMPTY / HR_ERROR_SEMAPHORE_FULL
HR_ERROR_MUTEX_BUSY
HR_ERROR_OVERFLOW
```

Status values are part of the contract. A timeout should not be changed into a panic, nor invalid context into silent success, without an explicit migration policy.

<a id="source-map"></a>
## Source map

- `kernel/include/hairtos/hr_diagnostics.h`
- `kernel/include/hairtos/hr_hooks.h`
- `kernel/src/hr_diagnostics.c`
- `arch/arm/cortex-m3/hr_fault.c`
- `arch/arm/cortex-m3/hr_faultasm.S`
- `tests/host/test_diagnostics.c`

<a id="references"></a>
## References

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Implementation sources in the repository:**
- `kernel/include/hairtos/hr_diagnostics.h`
- `kernel/include/hairtos/hr_hooks.h`
- `kernel/src/hr_diagnostics.c`
- `kernel/include/hairtos/hr_diagnostics.h`
- `arch/arm/cortex-m3/hr_fault.c`
- `arch/arm/cortex-m3/hr_faultasm.S`
- `tests/host/test_diagnostics.c`
