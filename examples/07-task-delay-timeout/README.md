# `07-task-delay-timeout` — SysTick, trì hoãn tác vụ và timeout

> **Môi trường:** Target. Target tham chiếu là `bluepill_f103c8`; target khác được chọn bằng `TARGET=<name>`.  
> **Vị trí mã nguồn:** `examples/07-task-delay-timeout/main.c`  
> **Mục đích:** SysTick trở thành kernel tick 1 kHz; task delay chuyển sang BLOCKED và được đánh thức từ timeout list.

## 1. Mục tiêu học tập

- Dùng `hr_task_delay()` để block tương đối.
- Dùng `hr_task_delay_until()` để chạy periodic không drift.
- Quan sát idle task chạy khi mọi application task đều BLOCKED.
- Hiểu timeout list và wake-up tại tick deadline.

## 2. Kiến thức trọng tâm

- SysTick do kernel quản lý.
- Chuyển trạng thái RUNNING → BLOCKED → READY.
- Dual timeout list hỗ trợ tick wrap.
- Example tắt general preemption và time slicing để tập trung vào delay.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `periodic` | Priority 2, stack 160 words | `delay_until` mỗi 500 ticks. |
| `heartbeat` | Priority 3, stack 160 words | `delay` mỗi 1000 ticks. |
| Idle task | Nội bộ | WFI khi cả hai task block. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Tần số tick | 1 kHz |
| Chu kỳ định kỳ | 500 ticks |
| Chu kỳ heartbeat | 1000 ticks |
| Preemption/time slicing | Tắt trong cấu hình example |

### Target và khả năng port

Application sử dụng public kernel/framework API và `board.h`. CPU flags, startup, linker script, port, tick IRQ, fault backend, driver và OpenOCD được lấy từ `cmake/targets/<target>.cmake`. Các chi tiết LED, UART, clock hoặc marker trong README là hành vi của target tham chiếu `bluepill_f103c8`; target khác phải cung cấp board service tương đương.

## 4. Luồng thực thi

1. Periodic task chạy và lưu release tick.
2. Heartbeat task chạy rồi cả hai gọi delay và chuyển BLOCKED.
3. Idle task chạy.
4. SysTick tăng tick và advance timeout list.
5. Đến deadline, task chuyển READY; nếu idle đang chạy, PendSV đưa task vừa thức dậy lên CPU.
6. Periodic dùng deadline trước làm mốc nên tránh drift.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hairtos/hr_time.h`
- `hairtos/hr_task.h`

### API trọng tâm

- `hr_time_now()`
- `hr_task_delay()`
- `hr_task_delay_until()`

### Module được đưa vào bản biên dịch

- `task_kernel`
- `kernel_runtime`
- `kernel_time`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make TARGET=bluepill_f103c8 EXAMPLE=07-task-delay-timeout build` |
| Flash và chạy | `make TARGET=bluepill_f103c8 EXAMPLE=07-task-delay-timeout run` |
| Kiểm tra | `make TARGET=bluepill_f103c8 EXAMPLE=07-task-delay-timeout check` |
| Dọn build riêng | `make TARGET=bluepill_f103c8 EXAMPLE=07-task-delay-timeout clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TARGET=bluepill_f103c8 TOOLCHAIN=clang EXAMPLE=07-task-delay-timeout build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos SysTick and task delay
periodic activation=1 tick=0 -> delay_until +500
heartbeat activation=1 tick=0 -> delay 1000
periodic activation=2 tick=500 -> delay_until +500
periodic activation=3 tick=1000 -> delay_until +500
heartbeat activation=2 tick=1000 -> delay 1000
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Periodic xuất hiện gần các tick bội 500.
- Heartbeat xuất hiện gần các tick bội 1000.
- Không có lỗi delay và hệ thống không busy-loop tại application level khi chờ.

### Lỗi thường gặp

- Task không thức dậy: kiểm tra SysTick handler và timeout advance.
- Periodic drift: kiểm tra `last_wake_tick` không bị gán lại theo thời điểm hoàn thành.
- Idle không chạy: kiểm tra task đã được loại khỏi ready queue khi block.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=07-task-delay-timeout clean
make TARGET=bluepill_f103c8 EXAMPLE=07-task-delay-timeout build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Chưa chứng minh CPU-bound tasks cùng priority được time slice.
- Chưa preempt general low task khi high task thức dậy trong mọi trường hợp.

- Khi chạy trên target khác, pin, clock, CPU name, marker và output phần cứng lấy từ board/target manifest; không nên xem giá trị của Blue Pill là contract chung.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`08-preemption-round-robin`](../08-preemption-round-robin/README.md). Bài tiếp theo bật preemption và time slicing bằng SysTick.
