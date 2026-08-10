# Status và kiểu dữ liệu

## `hr_status_t`

| Value | Ý nghĩa |
|---|---|
| `HR_OK` | Thành công |
| `HR_ERROR_INVALID_ARGUMENT` | NULL/range/size sai |
| `HR_ERROR_INVALID_STATE` | Object/state không cho phép operation |
| `HR_ERROR_TIMEOUT` | Blocking operation hết hạn |
| `HR_ERROR_QUEUE_FULL` | Queue send no-wait thất bại |
| `HR_ERROR_QUEUE_EMPTY` | Queue receive no-wait thất bại |
| `HR_ERROR_NO_MEMORY` | Static pool/table/task capacity hết |
| `HR_ERROR_NOT_OWNER` | Mutex unlock sai owner |
| `HR_ERROR_FROM_ISR` | Task-context API bị gọi từ ISR |
| `HR_ERROR_NOT_SUPPORTED` | Feature/context không hỗ trợ |
| `HR_ERROR_INTERNAL` | Internal invariant/operation lỗi |
| `HR_ERROR_SEMAPHORE_EMPTY` | Semaphore take no-wait thất bại |
| `HR_ERROR_SEMAPHORE_FULL` | Give vượt max |
| `HR_ERROR_MUTEX_BUSY` | Mutex không thể acquire ngay |
| `HR_ERROR_OVERFLOW` | Counter/reference overflow |

## Scalar types

```c
typedef uint32_t hr_tick_t;
typedef uint32_t hr_stack_t;
typedef uint8_t  hr_priority_t;
typedef uint32_t hr_irq_state_t;
```

Priority số nhỏ hơn là cao hơn.

## Opaque types

`hr_task_t`, `hr_queue_t`, `hr_semaphore_t`, `hr_mutex_t`, `hr_timer_t` là aligned byte storage. Không truy cập `.storage` trong application.

## Callback

```c
typedef void (*hr_task_entry_t)(void *);
typedef void (*hr_timer_callback_t)(void *);
```

Task entry không nên return.

## Timeout

```c
HR_NO_WAIT
HR_WAIT_FOREVER
```

Không phải mọi API đều chấp nhận cả hai; xem tài liệu API tương ứng.
