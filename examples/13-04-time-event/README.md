# `13-04-time-event` — Sự kiện thời gian haievent

> **Môi trường:** Target. Target tham chiếu là `bluepill_f103c8`; target khác được chọn bằng `TARGET=<name>`.  
> **Vị trí mã nguồn:** `examples/13-04-time-event/main.c`  
> **Mục đích:** Periodic time event dựa trên software timer post `SIGNAL_TICK` vào AO; state handler chạy trong AO task.

## 1. Mục tiêu học tập

- Tạo `he_time_event_t` tĩnh.
- Arm periodic event và disarm sau số lần xác định.
- Phân biệt timer-service callback với AO dispatch.
- Kết hợp timing và event-driven state handler.

## 2. Kiến thức trọng tâm

- Time event sở hữu một static event nội bộ.
- Software timer expiry chỉ post vào AO queue.
- AO xử lý event theo run-to-completion.
- Disarm ngăn deadline tiếp theo.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `blinker-AO` | Priority 2, stack 224, queue 6 | Toggle LED khi nhận tick. |
| `blink-time-event` | Chu kỳ 250 tick, tự động nạp lại | Đăng `SIGNAL_TICK`. |
| Timer service | Priority 1 | Chuyển expiration thành post operation. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Period | 250 ticks |
| Số event trước disarm | 6 |
| Tín hiệu | `SIGNAL_TICK` |

### Target và khả năng port

Application sử dụng public kernel/framework API và `board.h`. CPU flags, startup, linker script, port, tick IRQ, fault backend, driver và OpenOCD được lấy từ `cmake/targets/<target>.cmake`. Các chi tiết LED, UART, clock hoặc marker trong README là hành vi của target tham chiếu `bluepill_f103c8`; target khác phải cung cấp board service tương đương.

## 4. Luồng thực thi

1. Tạo AO và time event.
2. Kích hoạt time event trước khi khởi chạy kernel.
3. Mỗi 250 tick software timer hết hạn.
4. Dịch vụ timer đăng event vào queue của AO.
5. AO tăng count, toggle LED và in tick.
6. Tại count 6, AO disarm event và in PASS.

## 5. API và mã nguồn liên quan

### Header được dùng

- `haievent/haievent.h`
- `hairtos/hr_time.h`

### API trọng tâm

- `he_time_event_create_static()`
- `he_time_event_arm()`
- `he_time_event_disarm()`

### Module được đưa vào bản biên dịch

- `timer`
- `haievent`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make TARGET=bluepill_f103c8 EXAMPLE=13-04-time-event build` |
| Flash và chạy | `make TARGET=bluepill_f103c8 EXAMPLE=13-04-time-event run` |
| Kiểm tra | `make TARGET=bluepill_f103c8 EXAMPLE=13-04-time-event check` |
| Dọn build riêng | `make TARGET=bluepill_f103c8 EXAMPLE=13-04-time-event clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TARGET=bluepill_f103c8 TOOLCHAIN=clang EXAMPLE=13-04-time-event build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos time event
Timer service posts events; the AO dispatches them in task context.
blinker AO: waiting for periodic time events
time-event count=1 tick=250
...
time-event count=6 tick=1500
Time event: PASS
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Có đúng sáu event trước PASS.
- Tick tăng gần 250 mỗi event.
- Sau disarm không còn event mới.

### Lỗi thường gặp

- Event vẫn chạy sau khi disarm: timer chưa được gỡ hoặc trạng thái rearm sai.
- Callback gọi trực tiếp state: vi phạm yêu cầu chạy trong task context.
- Queue overflow: AO không consume đủ nhanh.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=13-04-time-event clean
make TARGET=bluepill_f103c8 EXAMPLE=13-04-time-event build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Không minh họa rearm hoặc đổi chu kỳ của time event; ví dụ software timer đã bao phủ thao tác đó.

- Khi chạy trên target khác, pin, clock, CPU name, marker và output phần cứng lấy từ board/target manifest; không nên xem giá trị của Blue Pill là contract chung.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`13-05-publish-subscribe`](../13-05-publish-subscribe/README.md). Bài tiếp theo multicast dynamic event và quản lý reference count.

### Liên hệ Version 2

Version 2 sẽ thêm trace dispatch/drop và bảo toàn time-event semantics trên tickless kernel.

Xem [`../../docs/09-version2/README.md`](../../docs/09-version2/README.md).
