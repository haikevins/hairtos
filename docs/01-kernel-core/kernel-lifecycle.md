# Kernel lifecycle

## 1. Mục tiêu

Định nghĩa trình tự hợp lệ từ reset đến task execution và trạng thái panic.

## 2. State machine

```text
RESET --hr_kernel_init()--> INITIALIZED --hr_kernel_start()--> RUNNING
  \-----------------------------------------------------------> PANIC
```

- `RESET`: global kernel chưa được khởi tạo.
- `INITIALIZED`: scheduler, all-task list, timeout list và idle task đã sẵn sàng.
- `RUNNING`: current task đã được chọn và CPU đã chuyển sang PSP.
- `PANIC`: kernel phát hiện lỗi không thể tiếp tục an toàn.

## 3. `hr_kernel_init()`

Hàm khởi tạo scheduler, timeout list, diagnostics tùy cấu hình, reset timer system và tạo idle task tĩnh. Idle task có priority `HR_CFG_IDLE_PRIORITY`, chạy vòng lặp `WFI`.

Chỉ được gọi một lần khi state là `RESET`.

## 4. Task registration

`hr_task_start()` gọi internal registration khi kernel đang `INITIALIZED`. Task chuyển `CREATED -> READY`, được thêm vào ready queue và all-task list. Application không được dùng idle priority.

## 5. `hr_kernel_start()`

Kernel chọn ready task priority cao nhất, chuyển nó sang `RUNNING`, cập nhật global current TCB và gọi port startup. Hàm không trở lại `main()` khi thành công.

## 6. Invariants

- Luôn có idle task READY hoặc RUNNING.
- Chỉ có tối đa một task RUNNING.
- `g_hr_current_task_control_block` phải trỏ đúng TCB của task hiện hành.
- Task count không vượt `HR_CFG_MAX_TASKS`.

## 7. Kiểm thử

Host test kiểm tra init/start state và first-task selection. Target Phase 4 kiểm tra SVC startup thật.

## 8. Giới hạn

Không hỗ trợ deinitialize hoặc restart kernel trong cùng boot.
