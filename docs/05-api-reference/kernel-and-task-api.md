# Kernel và Task API

## Kernel

### `hr_kernel_init()`

Context: trước scheduler, một lần/boot.

Tạo internal scheduler structures và idle task. Trả lỗi nếu lifecycle không hợp lệ.

### `hr_kernel_start()`

Context: sau init/start task.

Chọn first READY task rồi chuyển control qua port. Trên target thành công, hàm không return.

### Queries

```c
hr_kernel_is_running();
hr_kernel_get_state();
hr_kernel_get_tick();
hr_kernel_get_task_count();
```

## Task create

```c
hr_task_create_static(task, name, entry, arg,
                      stack, stack_words, priority);
```

Caller phải giữ `task` và `stack` tồn tại suốt runtime. Priority application không dùng idle priority.

## Task start

`hr_task_start()` chỉ register CREATED task. Không gọi hai lần.

## Queries

```c
hr_task_is_valid()
hr_task_get_name()
hr_task_get_state()
hr_task_get_base_priority()
hr_task_get_effective_priority()
hr_task_get_stack_words()
hr_task_get_stack_high_watermark()
hr_task_stack_guard_is_valid()
hr_task_current()
```

## Scheduling APIs

### `hr_task_yield()`

Task context. Chủ động nhường CPU; cùng priority có thể rotate.

### `hr_task_delay(ticks)`

- 0 → yield;
- finite >0 → BLOCKED;
- WAIT_FOREVER → invalid.

### `hr_task_delay_until(last, period)`

Periodic release. Caller giữ `last_wake_tick`.

## Administrative control

```c
hr_task_suspend(task);
hr_task_resume(task);
```

Suspend blocked task không hủy wait.

## Không hợp lệ trong ISR

Create/start/runtime blocking/admin APIs không phải ISR API.

## Task state

```text
CREATED -> READY -> RUNNING
              ^       |
              |       +-> BLOCKED
              |       +-> SUSPENDED
              +-------------+
```

Actual suspend/blocked interaction chi tiết xem kernel docs.
