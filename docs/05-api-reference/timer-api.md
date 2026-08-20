# Software Timer API

> **Scope:** Public API contracts for `hairtos 1.0.0-rc1`; internal helpers are not part of the compatibility surface.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](time-and-context-api.md)

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

Software timers do not execute callbacks in SysTick. The tick ISR only transfers expired timers to a pending queue and signals the timer-service task; callbacks execute in task context. Each timer tracks `pending_count` so multiple expirations are not silently collapsed while the service task is delayed.

- Public handles are opaque storage; application code must not cast them to internal TCB/control-block types.
- Functions return `hr_status_t` when an operation can fail; boolean/size/metadata queries return neutral values for invalid objects according to the current implementation.
- Blocking APIs are for task context only while the kernel is RUNNING; ISR variants are named `_from_isr` and never block.
- The caller retains ownership of static backing storage; create/init does not copy the entire object into a kernel heap.

<a id="functions"></a>
## Function surface

### `kernel/include/hairtos/hr_timer.h`

```c
hr_status_t hr_timer_create_static(hr_timer_t *timer, const char *name, hr_tick_t period_ticks, bool auto_reload, hr_timer_callback_t callback, void *argument);
bool hr_timer_is_valid(const hr_timer_t *timer);
bool hr_timer_is_active(const hr_timer_t *timer);
const char *hr_timer_get_name(const hr_timer_t *timer);
hr_tick_t hr_timer_get_period(const hr_timer_t *timer);
uint32_t hr_timer_get_pending_count(const hr_timer_t *timer);
hr_status_t hr_timer_start(hr_timer_t *timer);
hr_status_t hr_timer_stop(hr_timer_t *timer);
hr_status_t hr_timer_reset(hr_timer_t *timer);
hr_status_t hr_timer_change_period(hr_timer_t *timer, hr_tick_t new_period_ticks);
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

- `kernel/include/hairtos/hr_timer.h`
- `kernel/src/hr_timer.c`
- `kernel/internal/hr_timer_internal.h`
- `tests/host/test_timer.c`

<a id="references"></a>
## References

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Implementation sources in the repository:**
- `kernel/include/hairtos/hr_timer.h`
- `kernel/src/hr_timer.c`
- `kernel/internal/hr_timer_internal.h`
- `tests/host/test_timer.c`
