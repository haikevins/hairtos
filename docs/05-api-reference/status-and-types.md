# Status và kiểu dữ liệu

## 1. Header

```c
#include "hairtos/hr_status.h"
#include "hairtos/hr_types.h"
```

## 2. Scalar types

| Kiểu | Nền tảng | Ý nghĩa |
|---|---|---|
| `hr_tick_t` | `uint32_t` | Kernel tick/deadline |
| `hr_stack_t` | `uint32_t` | Stack word Cortex-M3 |
| `hr_priority_t` | `uint8_t` | Priority, số nhỏ hơn cao hơn |
| `hr_irq_state_t` | `uint32_t` | Trạng thái interrupt lưu khi enter critical |

## 3. Opaque object types

`hr_task_t`, `hr_queue_t`, `hr_semaphore_t`, `hr_mutex_t`, `hr_timer_t` là union static storage. Không truy cập field trực tiếp.

## 4. Callback types

```c
typedef void (*hr_task_entry_t)(void *argument);
typedef void (*hr_timer_callback_t)(void *argument);
```

## 5. Status codes

| Status | Ý nghĩa thường gặp |
|---|---|
| `HR_OK` | Thành công |
| `HR_ERROR_INVALID_ARGUMENT` | NULL, size/priority/range sai |
| `HR_ERROR_INVALID_STATE` | Object/state không cho phép operation |
| `HR_ERROR_TIMEOUT` | Blocking operation hết thời gian |
| `HR_ERROR_QUEUE_FULL/EMPTY` | Non-blocking queue không thực hiện được |
| `HR_ERROR_NO_MEMORY` | Static pool/table/task limit hết |
| `HR_ERROR_NOT_OWNER` | Unlock mutex không phải owner |
| `HR_ERROR_FROM_ISR` | API task-context bị gọi trong ISR |
| `HR_ERROR_NOT_SUPPORTED` | Feature/context chưa hỗ trợ |
| `HR_ERROR_INTERNAL` | Invariant nội bộ không đúng |
| `HR_ERROR_SEMAPHORE_EMPTY/FULL` | Take/give non-blocking thất bại |
| `HR_ERROR_MUTEX_BUSY` | Mutex không thể lock ngay |
| `HR_ERROR_OVERFLOW` | Counter/reference vượt giới hạn |

## 6. Quy tắc

Luôn kiểm tra return value của create/start/blocking API. Query API có thể trả 0/false khi object invalid; dùng `*_is_valid()` để phân biệt.
