# `13-06-event-driven-demo` — Demo haievent tích hợp

> **Môi trường:** Target — STM32F103C8T6  
> **Vị trí mã nguồn:** `examples/13-06-event-driven-demo/main.c`  
> **Mục đích:** Demo tích hợp controller state machine, heartbeat time event, dynamic status event, publish–subscribe và observer AO.

## 1. Mục tiêu học tập

- Kết hợp các capability haievent trong một flow hoàn chỉnh.
- Arm/disarm time event theo state ENTRY/EXIT.
- Publish dynamic status event từ state handler.
- Dùng script task để phát command START/STOP.

## 2. Kiến thức trọng tâm

- Controller có states IDLE và ACTIVE.
- Time event chỉ active khi controller ở ACTIVE.
- Heartbeat event nội bộ tạo status dynamic event.
- Observer subscribe `SIGNAL_STATUS`.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `controller-AO` | Priority 2, stack 256, queue 6 | State IDLE/ACTIVE và heartbeat count. |
| `observer-AO` | Priority 3, stack 256, queue 6 | Nhận status event. |
| `event-script` | Priority 4, stack 256 | START sau 200 ticks, STOP sau thêm 1400 ticks. |
| Heartbeat time event | 250 ticks periodic | Chỉ arm trong ACTIVE. |
| Event pool | 8 blocks × 64 bytes | Cấp status event. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Signals | START, STOP, HEARTBEAT, STATUS |
| Subscriber cho STATUS | Observer duy nhất |
| Publish mode | `HR_NO_WAIT`, yêu cầu delivered=1 |

## 4. Luồng thực thi

1. Controller start ở IDLE và tắt LED.
2. Script delay 200 rồi post START.
3. Transition tới ACTIVE: LED on và arm heartbeat time event.
4. Mỗi 250 tick controller nhận HEARTBEAT, cấp status event và publish.
5. Observer in heartbeat/tick và release event.
6. Sau 1400 ticks script post STOP.
7. ACTIVE EXIT disarm heartbeat rồi transition về IDLE.

## 5. API và mã nguồn liên quan

### Header được dùng

- `haievent/haievent.h`
- `hairtos/hr_time.h`

### API trọng tâm

- `he_state_transition()`
- `he_time_event_arm()`
- `he_time_event_disarm()`
- `he_event_new()`
- `he_pubsub_publish()`

### Module được đưa vào bản biên dịch

- `context`
- `queue`
- `semaphore`
- `timer`
- `haievent`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make EXAMPLE=13-06-event-driven-demo build` |
| Flash và chạy | `make EXAMPLE=13-06-event-driven-demo run` |
| Kiểm tra | `make EXAMPLE=13-06-event-driven-demo check` |
| Dọn build riêng | `make EXAMPLE=13-06-event-driven-demo clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TOOLCHAIN=clang EXAMPLE=13-06-event-driven-demo build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos haievent integration demo
controller: IDLE
script: START controller
controller: ACTIVE, arm heartbeat time event
observer: heartbeat=1 tick=<...>
observer: heartbeat=2 tick=<...>
script: STOP controller
controller: leave ACTIVE
controller: IDLE
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Heartbeat chỉ xuất hiện giữa START và STOP.
- Mỗi STATUS publish delivered đúng một observer.
- Sau STOP, time event dừng và controller trở lại IDLE.

### Lỗi thường gặp

- Heartbeat tiếp tục sau STOP: disarm ở EXIT thất bại.
- Pool cạn: status event ownership leak.
- Controller không transition: START/STOP signal hoặc queue sai.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make EXAMPLE=13-06-event-driven-demo clean
make EXAMPLE=13-06-event-driven-demo build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- State machine vẫn là flat, không hierarchical.
- Script chỉ chạy một chu kỳ START/STOP.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`14-memory-allocator-lab`](../14-memory-allocator-lab/README.md). Bài tiếp theo nghiên cứu allocator như một lab độc lập, không đưa dynamic heap vào kernel.
