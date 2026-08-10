# `13-01-event-post` — Đăng sự kiện haievent từ ISR

> **Môi trường:** Target. Target tham chiếu là `bluepill_f103c8`; target khác được chọn bằng `TARGET=<name>`.  
> **Vị trí mã nguồn:** `examples/13-01-event-post/main.c`  
> **Mục đích:** EXTI0 ISR post một static event vào Active Object; state handler chỉ chạy sau ISR trong task context.

## 1. Mục tiêu học tập

- Khởi tạo immutable static event.
- Post event từ ISR vào AO queue.
- Đánh thức AO priority cao và yield sau ISR.
- Giữ run-to-completion dispatch ngoài ISR.

## 2. Kiến thức trọng tâm

- Active Object = tác vụ + queue + máy trạng thái.
- Static event không cần release về pool.
- ISR chỉ enqueue; AO task dispatch.
- Higher-priority wake-up dùng `hr_yield_from_isr`.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `irq-receiver-AO` | Priority 2, stack 224, queue 4 | Nhận `SIGNAL_IRQ_SAMPLE`. |
| `irq-trigger` | Priority 4, stack 224 | Software-trigger EXTI0 mỗi 500 ticks. |
| Static event | `g_irq_event` | Immutable, dùng lại qua nhiều IRQ. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Tín hiệu | `HE_SIG_USER` |
| EXTI source | Software interrupt line 0 |
| Dung lượng queue | 4 |

### Target và khả năng port

Application sử dụng public kernel/framework API và `board.h`. CPU flags, startup, linker script, port, tick IRQ, fault backend, driver và OpenOCD được lấy từ `cmake/targets/<target>.cmake`. Các chi tiết LED, UART, clock hoặc marker trong README là hành vi của target tham chiếu `bluepill_f103c8`; target khác phải cung cấp board service tương đương.

## 4. Luồng thực thi

1. AO start và block trên event queue.
2. Trigger task delay 500 rồi ghi EXTI SWIER.
3. ISR post static event bằng `he_active_post_from_isr()`.
4. ISR request PendSV nếu AO priority cao hơn.
5. AO dispatch `receiver_state()` và in IRQ count/tick.
6. AO release ownership logic phù hợp static event rồi block lại.

## 5. API và mã nguồn liên quan

### Header được dùng

- `haievent/haievent.h`
- `hairtos/hr_context.h`

### API trọng tâm

- `he_event_init_static()`
- `he_active_create_static()`
- `he_active_post_from_isr()`
- `hr_yield_from_isr()`

### Module được đưa vào bản biên dịch

- `context`
- `queue`
- `timer`
- `haievent`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make TARGET=bluepill_f103c8 EXAMPLE=13-01-event-post build` |
| Flash và chạy | `make TARGET=bluepill_f103c8 EXAMPLE=13-01-event-post run` |
| Kiểm tra | `make TARGET=bluepill_f103c8 EXAMPLE=13-01-event-post check` |
| Dọn build riêng | `make TARGET=bluepill_f103c8 EXAMPLE=13-01-event-post clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TARGET=bluepill_f103c8 TOOLCHAIN=clang EXAMPLE=13-01-event-post build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos ISR event post
EXTI0 posts a static event; the AO dispatches after ISR return.
receiver AO: blocked on event queue
receiver AO: ISR event count=1 tick=500
receiver AO: ISR event count=2 tick=1000
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- IRQ count tăng và AO nhận đúng số event.
- State handler không chạy trong ISR.
- Không có post status lỗi hoặc queue overflow.

### Lỗi thường gặp

- Event mất: kiểm tra queue capacity, static event validity và ISR post.
- Handler chạy trong ISR: kiến trúc AO bị phá.
- AO không preempt trigger: kiểm tra higher-priority flag.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=13-01-event-post clean
make TARGET=bluepill_f103c8 EXAMPLE=13-01-event-post build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Truy cập EXTI trực tiếp chỉ phục vụ demo.

- Khi chạy trên target khác, pin, clock, CPU name, marker và output phần cứng lấy từ board/target manifest; không nên xem giá trị của Blue Pill là contract chung.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`13-02-active-object`](../13-02-active-object/README.md). Bài tiếp theo cho hai Active Object giao tiếp bằng event.

### Liên hệ Version 2

Version 2 nên thay demo IRQ phụ thuộc STM32F1 bằng target capability/board demo IRQ để example portable hơn.

Xem [`../../docs/09-version2/README.md`](../../docs/09-version2/README.md).
