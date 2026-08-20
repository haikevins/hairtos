# Kernel and Task API

> **Scope:** Public API contracts for `hairtos 1.0.0-rc1`; internal helpers are not part of the compatibility surface.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](haievent-api.md) · [Next →](mutex-api.md)

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

A task is an opaque public object with fixed-size storage, but internally it contains a full TCB: saved stack pointer, stack bounds, entry/argument, state, base/effective priority, ready/wait/timeout nodes, wait context, owned-mutex list, critical nesting, runtime counter, and magic value.

- Public handles are opaque storage; application code must not cast them to internal TCB/control-block types.
- Functions return `hr_status_t` when an operation can fail; boolean/size/metadata queries return neutral values for invalid objects according to the current implementation.
- Blocking APIs are for task context only while the kernel is RUNNING; ISR variants are named `_from_isr` and never block.
- The caller retains ownership of static backing storage; create/init does not copy the entire object into a kernel heap.

<a id="functions"></a>
## Function surface

### `kernel/include/hairtos/hr_kernel.h`

```c
hr_status_t hr_kernel_init(void);
hr_status_t hr_kernel_start(void);
bool hr_kernel_is_running(void);
hr_kernel_state_t hr_kernel_get_state(void);
hr_tick_t hr_kernel_get_tick(void);
size_t hr_kernel_get_task_count(void);
```

### `kernel/include/hairtos/hr_task.h`

```c
hr_status_t hr_task_create_static(hr_task_t *task, const char *name, hr_task_entry_t entry, void *argument, hr_stack_t *stack, size_t stack_words, hr_priority_t priority);
hr_status_t hr_task_start(hr_task_t *task);
bool hr_task_is_valid(const hr_task_t *task);
const char *hr_task_get_name(const hr_task_t *task);
hr_task_state_t hr_task_get_state(const hr_task_t *task);
hr_priority_t hr_task_get_base_priority(const hr_task_t *task);
hr_priority_t hr_task_get_effective_priority(const hr_task_t *task);
size_t hr_task_get_stack_words(const hr_task_t *task);
size_t hr_task_get_stack_high_watermark(const hr_task_t *task);
bool hr_task_stack_guard_is_valid(const hr_task_t *task);
hr_task_t *hr_task_current(void);
void hr_task_yield(void);
hr_status_t hr_task_delay(hr_tick_t ticks);
hr_status_t hr_task_delay_until(hr_tick_t *last_wake_tick, hr_tick_t period);
hr_status_t hr_task_suspend(hr_task_t *task);
hr_status_t hr_task_resume(hr_task_t *task);
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

- `kernel/include/hairtos/hr_kernel.h`
- `kernel/include/hairtos/hr_task.h`
- `kernel/src/hr_task.c`
- `kernel/internal/hr_task_internal.h`
- `kernel/src/hr_kernel.c`
- `tests/host/test_task.c`

<a id="references"></a>
## References

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Implementation sources in the repository:**
- `kernel/include/hairtos/hr_kernel.h`
- `kernel/include/hairtos/hr_task.h`
- `kernel/src/hr_task.c`
- `kernel/internal/hr_task_internal.h`
- `kernel/src/hr_kernel.c`
- `tests/host/test_task.c`
