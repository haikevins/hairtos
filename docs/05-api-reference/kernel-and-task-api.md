# Kernel và Task API

> **Phạm vi:** Public API contract của `hairtos 1.0.0-rc1`; internal helper không phải compatibility surface.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](haievent-api.md) · [Next →](mutex-api.md)

## Mục lục

- [Nguyên tắc API](#principles)
- [Function surface](#functions)
- [Context / blocking contract](#context)
- [Ownership và lifetime](#ownership)
- [Error semantics](#errors)
- [Source map](#source-map)
- [References](#references)

<a id="principles"></a>
## Nguyên tắc API

Task là opaque public object có storage cố định nhưng bên trong chứa TCB đầy đủ: saved stack pointer, stack bounds, entry/argument, state, base/effective priority, ready/wait/timeout nodes, wait context, owned-mutex list, critical nesting, runtime counter và magic.

- Public handles là opaque storage; không cast sang internal TCB/control block trong application.
- Function trả `hr_status_t` khi operation có thể fail; query bool/size/metadata dùng giá trị neutral nếu object invalid theo implementation hiện tại.
- API blocking chỉ dành cho task context khi kernel RUNNING; ISR variant được đặt tên `_from_isr` và không block.
- Caller giữ ownership của backing storage tĩnh; create/init không copy whole storage sang kernel heap.

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

| Nhóm | Task context | ISR context | Có thể block |
| --- | --- | --- | --- |
| Query/getter | Có | Chỉ khi implementation không cần blocking/lock dài | Không |
| API thường `send/take/lock/delay` | Có | Không | Có nếu timeout khác `HR_NO_WAIT` |
| API `_from_isr` | Không phải mục tiêu chính | Có | Không |
| `hr_critical_enter/exit` | Có | Có nhưng phải giữ cực ngắn | Không |

<a id="ownership"></a>
## Ownership và lifetime

- `hr_task_t` + task stack: caller-owned trong suốt lifetime task.
- Queue: caller-owned queue object + item storage.
- Semaphore/mutex/timer: caller-owned opaque object storage.
- haievent Active Object: caller-owned active storage + stack + event-pointer queue; dynamic event có reference count riêng.
- Không được move/free/reuse backing storage khi object còn valid/registered.

<a id="errors"></a>
## Error semantics

Các status public hiện có:

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

Status là một phần của contract. Không nên đổi một timeout thành panic hoặc một invalid-context thành silent success nếu chưa có migration policy.

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

**Nguồn implementation trong repository:**
- `kernel/include/hairtos/hr_kernel.h`
- `kernel/include/hairtos/hr_task.h`
- `kernel/src/hr_task.c`
- `kernel/internal/hr_task_internal.h`
- `kernel/src/hr_kernel.c`
- `tests/host/test_task.c`
