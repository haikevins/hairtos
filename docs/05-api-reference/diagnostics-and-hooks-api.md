# Diagnostics và Hooks API

> **Phạm vi:** Public API contract của `hairtos 1.0.0-rc1`; internal helper không phải compatibility surface.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [Next →](haievent-api.md)

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

Diagnostics v1 kết hợp runtime counters, task/stack snapshot, kernel invariant check và retained panic/fault record trong `.noinit`. Khi diagnostics bật, port còn enable Usage/Bus/Mem faults và trap unaligned/divide-by-zero để failure chuyển thành dữ liệu chẩn đoán thay vì silent corruption.

- Public handles là opaque storage; không cast sang internal TCB/control block trong application.
- Function trả `hr_status_t` khi operation có thể fail; query bool/size/metadata dùng giá trị neutral nếu object invalid theo implementation hiện tại.
- API blocking chỉ dành cho task context khi kernel RUNNING; ISR variant được đặt tên `_from_isr` và không block.
- Caller giữ ownership của backing storage tĩnh; create/init không copy whole storage sang kernel heap.

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

**Nguồn implementation trong repository:**
- `kernel/include/hairtos/hr_diagnostics.h`
- `kernel/include/hairtos/hr_hooks.h`
- `kernel/src/hr_diagnostics.c`
- `kernel/include/hairtos/hr_diagnostics.h`
- `arch/arm/cortex-m3/hr_fault.c`
- `arch/arm/cortex-m3/hr_faultasm.S`
- `tests/host/test_diagnostics.c`
