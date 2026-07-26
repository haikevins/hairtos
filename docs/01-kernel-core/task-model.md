# Task model

## 1. Mục tiêu

Mô tả TCB, state, priority, stack và wait metadata của task.

## 2. Public task object

`hr_task_t` là opaque storage. Application tạo task bằng `hr_task_create_static()` và đăng ký bằng `hr_task_start()`.

## 3. Internal TCB

Field đầu tiên là `stack_pointer`; assembly SVC/PendSV phụ thuộc offset 0 này. TCB còn chứa stack bounds, entry/argument, state, base/effective priority, time slice, intrusive nodes, wait context, owned mutex list, diagnostics counters và magic.

## 4. State model

```text
INVALID
  |
CREATED --start--> READY <---- timeout/event/resume ---- BLOCKED/SUSPENDED
                    |                                  ^
                    +---- selected ----> RUNNING ------+
```

State hợp lệ:

- `CREATED`: stack đã chuẩn bị nhưng chưa đăng ký.
- `READY`: nằm trong ready queue.
- `RUNNING`: task hiện hành.
- `BLOCKED`: chờ delay hoặc synchronization object.
- `SUSPENDED`: bị dừng hành chính, có thể lưu resume state READY hoặc BLOCKED.

## 5. Priority

- `base_priority`: priority cấu hình khi tạo task.
- `effective_priority`: có thể được boost bởi mutex priority inheritance.

Ready node và wait node phải phản ánh effective priority hiện tại.

## 6. Task return

Task entry phải chạy vô hạn hoặc tự xử lý termination logic. Nếu return, LR dẫn tới `hr_task_exit_error()`; hairtos không có API delete task.

## 7. Wait metadata

TCB lưu object đang chờ, wait list, buffer, cleanup callback, result và wait kind. Điều này cho phép timeout gỡ task khỏi đúng object và cho suspend giữ nguyên wait operation.

## 8. API liên quan

Xem `../05-api-reference/kernel-and-task-api.md`.

## 9. Giới hạn

Không có task deletion, join, notification hay affinity.
