# `13-02-active-object` — Active Object Ping–Pong

> **Môi trường:** Target — STM32F103C8T6  
> **Vị trí mã nguồn:** `examples/13-02-active-object/main.c`  
> **Mục đích:** Hai Active Object sở hữu task/queue/state riêng và trao đổi PING/PONG bằng static events.

## 1. Mục tiêu học tập

- Hiểu encapsulation của Active Object.
- Dùng state-machine context riêng cho mỗi actor.
- Post event giữa hai AO mà không chia sẻ control flow.
- Quan sát run-to-completion và queue-driven scheduling.

## 2. Kiến thức trọng tâm

- Mỗi AO có task, stack, queue và state machine.
- Static event PING/PONG được dùng lại.
- Context chứa peer, reply event và counter.
- Starter task chỉ kick-off chuỗi event.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `ping-AO` | Priority 2, stack 224, queue 4 | Handle PING rồi post PONG. |
| `pong-AO` | Priority 3, stack 224, queue 4 | Handle PONG rồi post PING. |
| `starter` | Priority 4, stack 224 | Post PING đầu tiên sau 100 ticks. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Signals | `SIGNAL_PING`, `SIGNAL_PONG` |
| Event memory | Static |
| Dispatch | Run-to-completion |

## 4. Luồng thực thi

1. Kernel start tạo hai AO tasks.
2. Mỗi AO nhận ENTRY và in started.
3. Starter post PING vào ping-AO.
4. Ping handle, tăng count, post PONG cho peer.
5. Pong handle, tăng count, post PING trở lại.
6. Chuỗi tiếp tục liên tục qua hai queue.

## 5. API và mã nguồn liên quan

### Header được dùng

- `haievent/haievent.h`

### API trọng tâm

- `he_active_create_static()`
- `he_active_post()`
- `he_state_machine_context()`

### Module được đưa vào build

- `queue`
- `timer`
- `haievent`

## 6. Build, run và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Build | `make EXAMPLE=13-02-active-object build` |
| Flash và chạy | `make EXAMPLE=13-02-active-object run` |
| Kiểm tra | `make EXAMPLE=13-02-active-object check` |
| Xóa build riêng | `make EXAMPLE=13-02-active-object clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TOOLCHAIN=clang EXAMPLE=13-02-active-object build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos active objects
ping-AO: Active Object started
pong-AO: Active Object started
starter: post first PING
ping-AO: handled event count=1
pong-AO: handled event count=1
ping-AO: handled event count=2
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Hai AO đều nhận ENTRY.
- Counters của ping và pong tăng luân phiên.
- Không có queue post failure.

### Lỗi thường gặp

- Chỉ một AO chạy: kiểm tra peer pointer hoặc reply event.
- Queue đầy nhanh: UART quá chậm hoặc producer loop không bị scheduling điều tiết.
- Context sai: kiểm tra `he_state_machine_context()`.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make EXAMPLE=13-02-active-object clean
make EXAMPLE=13-02-active-object build
```

## 9. Giới hạn của example

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Demo ping-pong không có delay sau kickoff nên UART traffic cao; đây không phải pattern production tối ưu.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`13-03-flat-state-machine`](../13-03-flat-state-machine/README.md). Bài tiếp theo tập trung vào semantics ENTRY/EXIT của state transition.
