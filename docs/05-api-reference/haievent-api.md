# haievent API

> **Phạm vi:** Public API contract của `hairtos 1.0.0-rc1`; internal helper không phải compatibility surface.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](diagnostics-and-hooks-api.md) · [Next →](kernel-and-task-api.md)

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

Active Object của v1 là composition cụ thể: một RTOS task, một queue chứa `he_event_t *`, một flat state machine và caller-owned storage. AO không thay scheduler; priority của AO chính là priority task mà kernel schedule.

- Public handles là opaque storage; không cast sang internal TCB/control block trong application.
- Function trả `hr_status_t` khi operation có thể fail; query bool/size/metadata dùng giá trị neutral nếu object invalid theo implementation hiện tại.
- API blocking chỉ dành cho task context khi kernel RUNNING; ISR variant được đặt tên `_from_isr` và không block.
- Caller giữ ownership của backing storage tĩnh; create/init không copy whole storage sang kernel heap.

<a id="functions"></a>
## Function surface

### `haievent/include/haievent/haievent.h`

File này chủ yếu export type/aggregate include; xem source header để biết exact declaration.

### `haievent/include/haievent/he_event.h`

```c
hr_status_t he_event_pool_init(he_event_pool_t *pool, void *storage, size_t block_size, size_t block_count);
bool he_event_pool_is_valid(const he_event_pool_t *pool);
size_t he_event_pool_get_free_count(const he_event_pool_t *pool);
size_t he_event_pool_get_block_count(const he_event_pool_t *pool);
he_event_t *he_event_new(he_event_pool_t *pool, he_signal_t signal, size_t event_size);
hr_status_t he_event_init_static(he_event_t *event, he_signal_t signal, size_t event_size);
hr_status_t he_event_retain(he_event_t *event);
hr_status_t he_event_release(he_event_t *event);
bool he_event_is_valid(const he_event_t *event);
bool he_event_is_dynamic(const he_event_t *event);
uint16_t he_event_get_reference_count(const he_event_t *event);
```

### `haievent/include/haievent/he_active.h`

```c
hr_status_t he_active_create_static(he_active_t *active, const char *name, he_state_handler_t initial_state, void *context, he_event_t **queue_storage, size_t queue_capacity, hr_stack_t *stack, size_t stack_words, hr_priority_t priority);
hr_status_t he_active_post(he_active_t *active, he_event_t *event, hr_tick_t timeout);
hr_status_t he_active_post_from_isr(he_active_t *active, he_event_t *event, bool *higher_priority_task_woken);
bool he_active_is_valid(const he_active_t *active);
const char *he_active_get_name(const he_active_t *active);
size_t he_active_get_pending_count(const he_active_t *active);
hr_task_t *he_active_get_task(he_active_t *active);
he_state_machine_t *he_active_get_state_machine(he_active_t *active);
```

### `haievent/include/haievent/he_state_machine.h`

```c
hr_status_t he_state_machine_init(he_state_machine_t *machine, he_state_handler_t initial_state, void *context);
hr_status_t he_state_machine_start(he_state_machine_t *machine);
hr_status_t he_state_machine_dispatch(he_state_machine_t *machine, const he_event_t *event);
he_state_result_t he_state_transition(he_state_machine_t *machine, he_state_handler_t target_state);
bool he_state_machine_is_valid(const he_state_machine_t *machine);
bool he_state_machine_is_started(const he_state_machine_t *machine);
he_state_handler_t he_state_machine_current(const he_state_machine_t *machine);
void *he_state_machine_context(he_state_machine_t *machine);
const void *he_state_machine_context_const(const he_state_machine_t *machine);
```

### `haievent/include/haievent/he_time_event.h`

```c
hr_status_t he_time_event_create_static(he_time_event_t *time_event, const char *name, he_active_t *target, he_signal_t signal, hr_tick_t period_ticks, bool periodic);
hr_status_t he_time_event_arm(he_time_event_t *time_event);
hr_status_t he_time_event_disarm(he_time_event_t *time_event);
hr_status_t he_time_event_rearm(he_time_event_t *time_event);
hr_status_t he_time_event_change_period(he_time_event_t *time_event, hr_tick_t period_ticks);
bool he_time_event_is_valid(const he_time_event_t *time_event);
bool he_time_event_is_armed(const he_time_event_t *time_event);
uint32_t he_time_event_get_dropped_count(const he_time_event_t *time_event);
```

### `haievent/include/haievent/he_pubsub.h`

```c
hr_status_t he_pubsub_init(he_pubsub_t *pubsub, he_active_t **subscriber_storage, size_t signal_count, size_t max_subscribers_per_signal);
bool he_pubsub_is_valid(const he_pubsub_t *pubsub);
hr_status_t he_pubsub_subscribe(he_pubsub_t *pubsub, he_signal_t signal, he_active_t *subscriber);
hr_status_t he_pubsub_unsubscribe(he_pubsub_t *pubsub, he_signal_t signal, he_active_t *subscriber);
size_t he_pubsub_get_subscriber_count(const he_pubsub_t *pubsub, he_signal_t signal);
hr_status_t he_pubsub_publish(he_pubsub_t *pubsub, he_event_t *event, hr_tick_t timeout, size_t *delivered_count);
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

- `haievent/include/haievent/haievent.h`
- `haievent/include/haievent/he_event.h`
- `haievent/include/haievent/he_active.h`
- `haievent/include/haievent/he_state_machine.h`
- `haievent/include/haievent/he_time_event.h`
- `haievent/include/haievent/he_pubsub.h`
- `haievent/src/he_active.c`
- `haievent/internal/he_internal.h`
- `haievent/src/he_state_machine.c`
- `kernel/src/hr_queue.c`

<a id="references"></a>
## References


**Nguồn implementation trong repository:**
- `haievent/include/haievent/haievent.h`
- `haievent/include/haievent/he_event.h`
- `haievent/include/haievent/he_active.h`
- `haievent/include/haievent/he_state_machine.h`
- `haievent/include/haievent/he_time_event.h`
- `haievent/include/haievent/he_pubsub.h`
- `haievent/src/he_active.c`
- `haievent/internal/he_internal.h`
- `haievent/src/he_state_machine.c`
- `kernel/src/hr_queue.c`
