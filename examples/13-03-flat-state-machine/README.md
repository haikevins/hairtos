# `13-03-flat-state-machine` — Máy trạng thái phẳng

> **Môi trường:** Target. Target tham chiếu là `bluepill_f103c8`; target khác được chọn bằng `TARGET=<name>`.  
> **Vị trí mã nguồn:** `examples/13-03-flat-state-machine/main.c`  
> **Mục đích:** Active Object điều khiển LED qua hai state OFF/ON và chuyển state theo static TOGGLE event.

## 1. Mục tiêu học tập

- Viết state handler trả về HANDLED/IGNORED/TRANSITION.
- Quan sát thứ tự EXIT → đổi current state → ENTRY.
- Kết hợp state machine với AO queue.
- Giữ state transition run-to-completion.

## 2. Kiến thức trọng tâm

- Flat state machine không có parent/child state.
- Reserved signals `HE_SIG_ENTRY` và `HE_SIG_EXIT`.
- `he_state_transition()` chỉ yêu cầu transition; framework thực hiện sequence.
- LED state phản ánh current state.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `switch-AO` | Priority 2, stack 224, queue 4 | State ban đầu OFF. |
| `toggle-controller` | Priority 3, stack 224 | Post sáu TOGGLE event mỗi 400 ticks. |
| Event | Static `SIGNAL_TOGGLE` | Điều khiển transition. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Số lần toggle | 6 |
| Chu kỳ | 400 ticks |
| Các trạng thái | `state_off`, `state_on` |

### Target và khả năng port

Application sử dụng public kernel/framework API và `board.h`. CPU flags, startup, linker script, port, tick IRQ, fault backend, driver và OpenOCD được lấy từ `cmake/targets/<target>.cmake`. Các chi tiết LED, UART, clock hoặc marker trong README là hành vi của target tham chiếu `bluepill_f103c8`; target khác phải cung cấp board service tương đương.

## 4. Luồng thực thi

1. AO start ở OFF và xử lý ENTRY: tắt LED.
2. Controller delay 400 và post TOGGLE.
3. OFF handler trả transition tới ON.
4. Framework gọi OFF EXIT, đổi state, gọi ON ENTRY.
5. Quá trình lặp sáu lần.
6. Controller in PASS và block định kỳ.

## 5. API và mã nguồn liên quan

### Header được dùng

- `haievent/haievent.h`

### API trọng tâm

- `he_state_transition()`
- `he_active_post()`
- `he_event_init_static()`

### Module được đưa vào bản biên dịch

- `queue`
- `timer`
- `haievent`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make TARGET=bluepill_f103c8 EXAMPLE=13-03-flat-state-machine build` |
| Flash và chạy | `make TARGET=bluepill_f103c8 EXAMPLE=13-03-flat-state-machine run` |
| Kiểm tra | `make TARGET=bluepill_f103c8 EXAMPLE=13-03-flat-state-machine check` |
| Dọn build riêng | `make TARGET=bluepill_f103c8 EXAMPLE=13-03-flat-state-machine clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TARGET=bluepill_f103c8 TOOLCHAIN=clang EXAMPLE=13-03-flat-state-machine build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos flat state machine
state OFF: ENTRY
state OFF: EXIT
state ON: ENTRY
state ON: EXIT
state OFF: ENTRY
...
Flat state-machine ENTRY/EXIT transition demo: PASS
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Mỗi transition có EXIT trước ENTRY.
- LED khớp state ON/OFF.
- Có PASS sau sáu toggle.

### Lỗi thường gặp

- ENTRY thiếu: framework transition sequence sai.
- LED/state lệch: logic handler hoặc active-low board API.
- Event ignored: signal value không khớp.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=13-03-flat-state-machine clean
make TARGET=bluepill_f103c8 EXAMPLE=13-03-flat-state-machine build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Chỉ flat state machine; không có hierarchical state, history hoặc guard/action riêng.

- Khi chạy trên target khác, pin, clock, CPU name, marker và output phần cứng lấy từ board/target manifest; không nên xem giá trị của Blue Pill là contract chung.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`13-04-time-event`](../13-04-time-event/README.md). Bài tiếp theo dùng software timer để post event định kỳ vào AO.

### Liên hệ Version 2

Flat FSM phải tiếp tục hoạt động như subset/compatibility layer của HSM engine Version 2.

Xem [`../../docs/09-version2/README.md`](../../docs/09-version2/README.md).
