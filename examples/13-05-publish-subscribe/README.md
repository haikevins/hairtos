# `13-05-publish-subscribe` — Publish–Subscribe và quyền sở hữu sự kiện động

> **Môi trường:** Target. Target tham chiếu là `bluepill_f103c8`; target khác được chọn bằng `TARGET=<name>`.  
> **Vị trí mã nguồn:** `examples/13-05-publish-subscribe/main.c`  
> **Mục đích:** Publisher cấp phát một telemetry event từ fixed-block pool và multicast tới logger/display subscribers.

## 1. Mục tiêu học tập

- Khởi tạo event pool không dùng malloc.
- Đăng ký nhiều subscriber theo signal.
- Publish cùng một event tới nhiều AO.
- Theo dõi reference count và trả block về pool sau subscriber cuối.

## 2. Kiến thức trọng tâm

- Dynamic event có header `he_event_t` và payload mở rộng.
- Publisher chuyển ownership cho bus.
- Bus retain một reference cho mỗi delivery thành công.
- Mỗi AO release event sau dispatch.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| Event pool | 6 blocks × 64 bytes | Cấp `telemetry_event_t`. |
| Pub/sub bus | 64 signals × tối đa 2 subscriber | Routing theo signal. |
| `logger-AO` | Priority 2, stack 224, queue 4 | Subscriber thứ nhất. |
| `display-AO` | Priority 3, stack 224, queue 4 | Subscriber thứ hai. |
| `publisher` | Priority 4, stack 224 | Publish mỗi 500 ticks. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Tín hiệu | `SIGNAL_TELEMETRY` |
| Số subscriber | 2 |
| Timeout publish | `HR_WAIT_FOREVER` |

### Target và khả năng port

Application sử dụng public kernel/framework API và `board.h`. CPU flags, startup, linker script, port, tick IRQ, fault backend, driver và OpenOCD được lấy từ `cmake/targets/<target>.cmake`. Các chi tiết LED, UART, clock hoặc marker trong README là hành vi của target tham chiếu `bluepill_f103c8`; target khác phải cung cấp board service tương đương.

## 4. Luồng thực thi

1. Publisher xin block từ pool và ghi sequence.
2. Bus tra subscriber list của signal.
3. Event được retain và post tới logger/display.
4. Publisher in `delivered=2`.
5. Mỗi AO đọc payload, tăng count và dispatch hoàn tất.
6. Sau reference cuối, block quay về pool; chu kỳ lặp sau 500 ticks.

## 5. API và mã nguồn liên quan

### Header được dùng

- `haievent/haievent.h`

### API trọng tâm

- `he_event_pool_init()`
- `he_event_new()`
- `he_pubsub_init()`
- `he_pubsub_subscribe()`
- `he_pubsub_publish()`

### Module được đưa vào bản biên dịch

- `queue`
- `timer`
- `haievent`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make TARGET=bluepill_f103c8 EXAMPLE=13-05-publish-subscribe build` |
| Flash và chạy | `make TARGET=bluepill_f103c8 EXAMPLE=13-05-publish-subscribe run` |
| Kiểm tra | `make TARGET=bluepill_f103c8 EXAMPLE=13-05-publish-subscribe check` |
| Dọn build riêng | `make TARGET=bluepill_f103c8 EXAMPLE=13-05-publish-subscribe clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TARGET=bluepill_f103c8 TOOLCHAIN=clang EXAMPLE=13-05-publish-subscribe build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos publish/subscribe
publisher: delivered=2 sequence=1
logger: telemetry sequence=1 count=1
display: telemetry sequence=1 count=1
publisher: delivered=2 sequence=2
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Mỗi publish delivered=2.
- Cả logger và display nhận cùng sequence.
- Pool không cạn sau nhiều chu kỳ.

### Lỗi thường gặp

- Pool cạn: reference leak hoặc event không release.
- Delivered < 2: subscribe table/queue post lỗi.
- Subscriber payload sai: cast/size dynamic event không đúng.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=13-05-publish-subscribe clean
make TARGET=bluepill_f103c8 EXAMPLE=13-05-publish-subscribe build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Không minh họa unsubscribe hoặc partial delivery do queue full.

- Khi chạy trên target khác, pin, clock, CPU name, marker và output phần cứng lấy từ board/target manifest; không nên xem giá trị của Blue Pill là contract chung.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`13-06-event-driven-demo`](../13-06-event-driven-demo/README.md). Bài tiếp theo tích hợp state machine, time event, dynamic event và pub/sub.
