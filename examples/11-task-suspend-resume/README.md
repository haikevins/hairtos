# `11-task-suspend-resume` — Tạm dừng và tiếp tục tác vụ

> **Môi trường:** Target — STM32F103C8T6  
> **Vị trí mã nguồn:** `examples/11-task-suspend-resume/main.c`  
> **Mục đích:** Suspend/resume task READY, RUNNING hoặc BLOCKED, đồng thời bảo toàn wait/timeout state và preempt khi resume high task.

## 1. Mục tiêu học tập

- Suspend một task đang BLOCKED bởi delay.
- Cho timeout hoàn tất nhưng không đưa task suspended vào ready queue.
- Resume high task và quan sát preemption tức thời.
- Self-suspend rồi resume từ supervisor.

## 2. Kiến thức trọng tâm

- Administrative suspension tách biệt với wait reason.
- SUSPENDED(BLOCKED) và SUSPENDED(READY).
- Resume phục hồi BLOCKED nếu event chưa hoàn tất, hoặc READY nếu đã hoàn tất.
- Không cho suspend idle task.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `worker` | Priority 1, stack 224 words | Delay 100, bị suspend, sau đó self-suspend. |
| `supervisor` | Priority 2, stack 224 words | Điều khiển suspend/resume. |
| `background` | Priority 4, stack 224 words | CPU workload và LED activity. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Suspend worker | Tick khoảng 50 |
| Resume lần 1 | Tick khoảng 250 |
| Resume lần 2 | Sau 100 ticks |

## 4. Luồng thực thi

1. Worker gọi delay 100 và chuyển BLOCKED.
2. Supervisor delay 50 rồi suspend worker đang BLOCKED.
3. Worker timeout ở tick 100 nhưng vẫn SUSPENDED.
4. Supervisor chờ thêm 200 ticks và xác nhận worker chưa READY.
5. Supervisor tiếp tục worker; worker priority 1 chiếm quyền ngay.
6. Worker in trạng thái resume rồi tự suspend.
7. Supervisor resume lần hai; worker tiếp tục ngay sau lời gọi self-suspend.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hairtos/hr_task.h`
- `hairtos/hr_time.h`

### API trọng tâm

- `hr_task_suspend()`
- `hr_task_resume()`
- `hr_task_get_state()`
- `hr_task_delay()`

### Module được đưa vào bản biên dịch

- `task_kernel`
- `kernel_runtime`
- `kernel_time`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make EXAMPLE=11-task-suspend-resume build` |
| Flash và chạy | `make EXAMPLE=11-task-suspend-resume run` |
| Kiểm tra | `make EXAMPLE=11-task-suspend-resume check` |
| Dọn build riêng | `make EXAMPLE=11-task-suspend-resume clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TOOLCHAIN=clang EXAMPLE=11-task-suspend-resume build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos task suspend and resume
worker: delay 100 ticks
supervisor: suspend blocked worker at tick=50
supervisor: resume high-priority worker at tick=250
worker: timeout completed while suspended; resumed at tick=250
worker: self-suspend
supervisor: resume self-suspended worker
worker: self-resume PASS at tick=<...>
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Timeout không tự làm worker READY khi đang suspended.
- Worker preempt supervisor ngay sau resume lần 1.
- Self-suspend trả về đúng vị trí sau resume lần 2.

### Lỗi thường gặp

- Worker READY trước resume: suspend overlay không được bảo toàn.
- Không preempt sau resume: scheduler không xét effective priority.
- Resume ở trạng thái không hợp lệ: kiểm tra trạng thái tác vụ trước lời gọi.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make EXAMPLE=11-task-suspend-resume clean
make EXAMPLE=11-task-suspend-resume build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Không có public resume-from-ISR trong example này.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`12-software-timer`](../12-software-timer/README.md). Bài tiếp theo đưa callback định thời vào timer-service task.
