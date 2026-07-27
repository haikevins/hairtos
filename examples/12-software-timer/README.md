# `12-software-timer` — Dịch vụ bộ định thời phần mềm

> **Môi trường:** Target. Target tham chiếu là `bluepill_f103c8`; target khác được chọn bằng `TARGET=<name>`.  
> **Vị trí mã nguồn:** `examples/12-software-timer/main.c`  
> **Mục đích:** One-shot và periodic software timer được quản lý bằng kernel tick; callback chạy trong timer-service task context.

## 1. Mục tiêu học tập

- Tạo timer tĩnh.
- Start, reset, change period và stop timer.
- Phân biệt timer expiration trong SysTick với callback execution trong task.
- Kiểm tra one-shot chỉ callback một lần và periodic tự rearm.

## 2. Kiến thức trọng tâm

- Danh sách deadline của timer được sắp thứ tự.
- Pending callback list và timer-service semaphore.
- Callback không chạy trong ISR.
- Periodic rearm từ deadline để hạn chế drift.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `timer-control` | Priority 3, stack 224 words | Điều khiển start/reset/stop. |
| `periodic` | 250 ticks → 500 ticks, auto reload | Toggle LED và đếm callback. |
| `one-shot` | 1000 ticks, one shot | Được reset tại tick 400 nên deadline thành 1400. |
| Timer service | Priority 1 theo build config | Chạy callback trong task context. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Software timer | Bật |
| Chu kỳ ban đầu | 250 ticks |
| One-shot | 1000 ticks |
| Đối số one-shot | 120012 |

### Target và khả năng port

Application sử dụng public kernel/framework API và `board.h`. CPU flags, startup, linker script, port, tick IRQ, fault backend, driver và OpenOCD được lấy từ `cmake/targets/<target>.cmake`. Các chi tiết LED, UART, clock hoặc marker trong README là hành vi của target tham chiếu `bluepill_f103c8`; target khác phải cung cấp board service tương đương.

## 4. Luồng thực thi

1. Control task start cả hai timer.
2. Periodic callback chạy ở các deadline 250, 500, 750, 1000.
3. Control reset one-shot ở tick 400, deadline mới khoảng 1400.
4. Callback periodic lần 4 đổi period thành 500 ticks.
5. Sau thời gian quan sát, control xác nhận callback count.
6. Stop periodic và in PASS.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hairtos/hr_timer.h`
- `hairtos/hr_time.h`

### API trọng tâm

- `hr_timer_create_static()`
- `hr_timer_start()`
- `hr_timer_reset()`
- `hr_timer_change_period()`
- `hr_timer_stop()`

### Module được đưa vào bản biên dịch

- `task_kernel`
- `kernel_runtime`
- `kernel_time`
- `semaphore`
- `timer`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make TARGET=bluepill_f103c8 EXAMPLE=12-software-timer build` |
| Flash và chạy | `make TARGET=bluepill_f103c8 EXAMPLE=12-software-timer run` |
| Kiểm tra | `make TARGET=bluepill_f103c8 EXAMPLE=12-software-timer check` |
| Dọn build riêng | `make TARGET=bluepill_f103c8 EXAMPLE=12-software-timer clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TARGET=bluepill_f103c8 TOOLCHAIN=clang EXAMPLE=12-software-timer build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos software timer
control: start periodic 250-tick and one-shot 1000-tick timers
timer-service: periodic callback=1 tick=250
control: reset one-shot; deadline moves 1000 -> 1400 ticks
timer-service: change periodic period 250 -> 500 ticks
timer-service: one-shot callback tick=1400 argument=120012
Software timer service: PASS
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- One-shot callback đúng một lần.
- Periodic callback ít nhất sáu lần và period đổi sau callback 4.
- Callback chạy khi current context là timer-service task, không phải SysTick.

### Lỗi thường gặp

- Callback không chạy: kiểm tra timer tick, pending list và service semaphore.
- One-shot chạy hai lần: auto_reload/state sai.
- Reset không dời deadline: kiểm tra remove/reinsert timer node.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=12-software-timer clean
make TARGET=bluepill_f103c8 EXAMPLE=12-software-timer build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Không có timer command API từ ISR trong example.

- Khi chạy trên target khác, pin, clock, CPU name, marker và output phần cứng lấy từ board/target manifest; không nên xem giá trị của Blue Pill là contract chung.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`13-01-event-post`](../13-01-event-post/README.md). Nhóm bài tiếp theo xây haievent trên queue, task và software timer.
