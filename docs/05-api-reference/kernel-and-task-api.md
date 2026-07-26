# Kernel và task API

## 1. Header

```c
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
```

## 2. Kernel API

| Hàm | Ngữ cảnh | Mô tả |
|---|---|---|
| `hr_kernel_init()` | trước scheduler | Khởi tạo kernel và idle task |
| `hr_kernel_start()` | trước scheduler | Chọn first task và không trở lại khi thành công |
| `hr_kernel_is_running()` | task/diagnostics | Kiểm tra RUNNING |
| `hr_kernel_get_state()` | mọi task context | Lấy lifecycle state |
| `hr_kernel_get_tick()` | task | Lấy tick |
| `hr_kernel_get_task_count()` | task | Số task đã đăng ký |

## 3. Task creation

```c
hr_status_t hr_task_create_static(
    hr_task_t *task,
    const char *name,
    hr_task_entry_t entry,
    void *argument,
    hr_stack_t *stack,
    size_t stack_words,
    hr_priority_t priority);
```

Yêu cầu stack ít nhất `HR_CFG_MIN_TASK_STACK_WORDS`; priority phải nhỏ hơn idle priority cho application.

`hr_task_start()` chỉ hợp lệ khi kernel INITIALIZED và task CREATED.

## 4. Scheduling/time API

- `hr_task_yield()` cooperative yield.
- `hr_task_delay(ticks)` blocking delay; ticks không được 0 hoặc WAIT_FOREVER.
- `hr_task_delay_until(last, period)` periodic release.

## 5. Administration

- `hr_task_suspend(task)`.
- `hr_task_resume(task)`.

Chỉ task context; idle không được suspend.

## 6. Queries

Name, state, base/effective priority, stack size, high-watermark, guard và current task.

## 7. Ví dụ tối thiểu

```c
static hr_task_t worker;
static hr_stack_t worker_stack[256];

static void worker_entry(void *arg)
{
    (void)arg;
    for (;;)
    {
        /* work */
        (void)hr_task_delay(100U);
    }
}

hr_kernel_init();
hr_task_create_static(&worker, "worker", worker_entry, NULL,
                      worker_stack, 256U, 2U);
hr_task_start(&worker);
hr_kernel_start();
```

## 8. Lưu ý

Task entry không được return. Không gọi blocking API trong ISR.
