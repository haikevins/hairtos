# `01-baremetal-foundation` — Bare-metal Foundation

> **Môi trường:** Target — STM32F103C8T6  
> **Vị trí mã nguồn:** `examples/01-baremetal-foundation/main.c`  
> **Mục đích:** Thiết lập nền tảng bare-metal trước khi đưa kernel vào hệ thống: clock, GPIO, UART, SysTick tạm thời và vòng lặp chính.

## 1. Mục tiêu học tập

- Khởi tạo Blue Pill và xác nhận clock hệ thống hoạt động.
- Điều khiển LED PC13 bằng API board.
- Gửi log qua USART1 và theo dõi thời gian bằng bộ đếm millisecond bare-metal.
- Hiểu vòng lặp super-loop và giới hạn của busy-wait trước khi có scheduler.

## 2. Kiến thức trọng tâm

- Startup STM32F103, sao chép `.data` và xóa `.bss`.
- HSE 8 MHz → PLL x9 → 72 MHz; có cơ chế fallback về HSI trong platform.
- GPIO output active-low, UART polling và SysTick bare-metal.
- Không có TCB, scheduler, PSP, SVC hoặc PendSV.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| Main loop | `heartbeat` và `board_millis()` | Toggle LED, tăng bộ đếm và delay 500 ms. |
| Tick source | `drivers/common/hr_systick_baremetal_irq.c` | Cung cấp millisecond counter trước khi SysTick thuộc quyền kernel. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Chu kỳ LED/log | 500 ms |
| Clock mục tiêu | 72 MHz khi HSE hoạt động |
| Kernel | Không sử dụng |

## 4. Luồng thực thi

1. `board_init()` cấu hình clock, GPIO, UART và tick bare-metal.
2. Vòng lặp toggle LED PC13.
3. Tăng `heartbeat` và in cùng `board_millis()`.
4. `board_delay_ms(500)` tạo khoảng chờ bận.
5. Quay lại đầu vòng lặp.

## 5. API và mã nguồn liên quan

### Header được dùng

- `board.h`

### API trọng tâm

- `board_init()`
- `board_led_toggle()`
- `board_uart_write_*()`
- `board_millis()`
- `board_delay_ms()`

### Module được đưa vào build

- `platform`
- `baremetal_tick`

## 6. Build, run và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Build | `make EXAMPLE=01-baremetal-foundation build` |
| Flash và chạy | `make EXAMPLE=01-baremetal-foundation run` |
| Kiểm tra | `make EXAMPLE=01-baremetal-foundation check` |
| Xóa build riêng | `make EXAMPLE=01-baremetal-foundation clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TOOLCHAIN=clang EXAMPLE=01-baremetal-foundation build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
Bare-metal foundation ready.
LED PC13 toggles every 500 ms.
heartbeat=1 uptime_ms=0
heartbeat=2 uptime_ms=500
heartbeat=3 uptime_ms=1000
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- LED PC13 đổi trạng thái xấp xỉ mỗi 500 ms.
- `heartbeat` tăng liên tục và `uptime_ms` không giảm.
- Không rơi vào `board_panic()` và UART không xuất ký tự rác.

### Lỗi thường gặp

- Không có UART: kiểm tra PA9/GND, baud 115200 và clock.
- LED không nháy: nhớ PC13 active-low và kiểm tra board clone.
- Thời gian sai rõ rệt: kiểm tra HSE/HSI và `board_get_core_clock_hz()` trong platform.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make EXAMPLE=01-baremetal-foundation clean
make EXAMPLE=01-baremetal-foundation build
```

## 9. Giới hạn của example

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Delay vẫn là busy-wait; CPU không thể chạy công việc khác trong thời gian chờ.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`02-kernel-data-structures-host`](../02-kernel-data-structures-host/README.md). Bài tiếp theo tách các cấu trúc dữ liệu scheduler khỏi phần cứng và kiểm chứng chúng trên host.
