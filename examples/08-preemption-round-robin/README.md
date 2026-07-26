# `08-preemption-round-robin` — Chiếm quyền và Round-Robin

> **Môi trường:** Target — STM32F103C8T6  
> **Vị trí mã nguồn:** `examples/08-preemption-round-robin/main.c`  
> **Mục đích:** Hai worker CPU-bound không gọi yield vẫn chia sẻ CPU; monitor priority cao được đánh thức định kỳ và preempt ngay.

## 1. Mục tiêu học tập

- Chứng minh preemption khi task priority cao chuyển READY.
- Chứng minh time slicing giữa hai task cùng priority.
- Phát hiện starvation bằng cách so sánh worker counters.
- Giữ PendSV là nơi duy nhất save/restore context.

## 2. Kiến thức trọng tâm

- SysTick quyết định PREEMPT hoặc TIME_SLICE rồi pend PendSV.
- Monitor priority 1 cao hơn worker priority 3.
- Worker không gọi kernel API trong vòng lặp.
- Round-robin dùng quantum `HR_CFG_TIME_SLICE_TICKS`.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `monitor` | Priority 1, stack 192 words | Chạy mỗi 250 ticks và kiểm tra counters. |
| `worker-a` | Priority 3, stack 192 words | CPU-bound counter. |
| `worker-b` | Priority 3, stack 192 words | CPU-bound counter. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Monitor period | 250 ticks |
| Worker priority | 3 |
| Preemption | Bật |
| Time slicing | Bật |

## 4. Luồng thực thi

1. Monitor chạy, chụp counters rồi block đến release tiếp theo.
2. Worker A/B chiếm CPU.
3. Mỗi quantum SysTick rotate worker queue và pend PendSV.
4. Khi monitor hết delay, SysTick nhận thấy priority 1 cao hơn current worker và yêu cầu preemption.
5. Monitor xác nhận cả hai counter đều tăng; nếu một counter đứng yên thì panic.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hairtos/hr_time.h`
- `hr_port.h`

### API trọng tâm

- `hr_task_delay_until()`
- `hr_task_current()`

### Module được đưa vào bản biên dịch

- `task_kernel`
- `kernel_runtime`
- `kernel_time`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make EXAMPLE=08-preemption-round-robin build` |
| Flash và chạy | `make EXAMPLE=08-preemption-round-robin run` |
| Kiểm tra | `make EXAMPLE=08-preemption-round-robin check` |
| Dọn build riêng | `make EXAMPLE=08-preemption-round-robin clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TOOLCHAIN=clang EXAMPLE=08-preemption-round-robin build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos preemption and round-robin
Two CPU-bound equal-priority workers never call yield().
SysTick round-robin shares CPU; monitor wake-up preempts them.
monitor preempted workers at tick=0 worker-a=0 worker-b=0
monitor preempted workers at tick=250 worker-a=<tăng> worker-b=<tăng>
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Sau activation đầu, cả hai worker counter đều tăng giữa hai report.
- Monitor chạy gần mỗi 250 tick.
- Không xuất hiện starvation error.

### Lỗi thường gặp

- Một counter đứng yên: kiểm tra quantum expiry và rotate highest queue.
- Monitor chạy muộn lâu: kiểm tra higher-priority preemption.
- Context hỏng: kiểm tra atomic selector và PendSV masking.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make EXAMPLE=08-preemption-round-robin clean
make EXAMPLE=08-preemption-round-robin build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Không kiểm soát chính xác tỷ lệ CPU bằng UART log; dùng benchmark/logic analyzer cho phép đo định lượng.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`09-queue-blocking-ipc`](../09-queue-blocking-ipc/README.md). Bài tiếp theo thêm IPC queue và blocking timeout.
