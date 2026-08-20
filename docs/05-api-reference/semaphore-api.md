# Semaphore API

> **Phạm vi:** Public API contract của `hairtos 1.0.0-rc1`; internal helper không phải compatibility surface.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](queue-api.md) · [Next →](status-and-types.md)

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

Semaphore là synchronization counter không có ownership. Counting semaphore giữ `count/max_count`; binary semaphore chỉ là cấu hình max=1. `give` ưu tiên đánh thức waiter thay vì tăng count khi có task đang chờ.

- Public handles là opaque storage; không cast sang internal TCB/control block trong application.
- Function trả `hr_status_t` khi operation có thể fail; query bool/size/metadata dùng giá trị neutral nếu object invalid theo implementation hiện tại.
- API blocking chỉ dành cho task context khi kernel RUNNING; ISR variant được đặt tên `_from_isr` và không block.
- Caller giữ ownership của backing storage tĩnh; create/init không copy whole storage sang kernel heap.

<a id="functions"></a>
## Function surface

### `kernel/include/hairtos/hr_semaphore.h`

```c
hr_status_t hr_semaphore_create_counting(hr_semaphore_t *semaphore, uint32_t initial_count, uint32_t max_count);
hr_status_t hr_semaphore_create_binary(hr_semaphore_t *semaphore, bool initially_available);
bool hr_semaphore_is_valid(const hr_semaphore_t *semaphore);
uint32_t hr_semaphore_get_count(const hr_semaphore_t *semaphore);
uint32_t hr_semaphore_get_max_count(const hr_semaphore_t *semaphore);
size_t hr_semaphore_get_waiting_tasks(const hr_semaphore_t *semaphore);
hr_status_t hr_semaphore_take(hr_semaphore_t *semaphore, hr_tick_t timeout);
hr_status_t hr_semaphore_give(hr_semaphore_t *semaphore);
hr_status_t hr_semaphore_give_from_isr(hr_semaphore_t *semaphore, bool *higher_priority_task_woken);
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

- `kernel/include/hairtos/hr_semaphore.h`
- `kernel/src/hr_semaphore.c`
- `kernel/internal/hr_semaphore_internal.h`
- `tests/host/test_semaphore.c`

<a id="references"></a>
## References

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Nguồn implementation trong repository:**
- `kernel/include/hairtos/hr_semaphore.h`
- `kernel/src/hr_semaphore.c`
- `kernel/internal/hr_semaphore_internal.h`
- `tests/host/test_semaphore.c`
