# `09-queue-blocking-ipc` — Queue và IPC chặn

> **Môi trường:** Target. Target tham chiếu là `bluepill_f103c8`; target khác được chọn bằng `TARGET=<name>`.  
> **Vị trí mã nguồn:** `examples/09-queue-blocking-ipc/main.c`  
> **Mục đích:** Producer và consumer trao đổi message qua queue FIFO tĩnh, có blocking, timeout, direct handoff và preemption.

## 1. Mục tiêu học tập

- Tạo queue với storage do application cấp.
- Block receiver khi queue rỗng và sender khi queue đầy.
- Dùng finite timeout cho send.
- Xác nhận FIFO của các message được nhận thành công.

## 2. Kiến thức trọng tâm

- Ring buffer với head/tail/count.
- Các wait list gửi/nhận được sắp theo priority.
- Direct handoff tới blocked receiver.
- Refill slot từ blocked sender khi receiver lấy item.
- Timeout cleanup khỏi queue wait list và timeout list.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| Queue | 2 phần tử `queue_message_t` | Mỗi message chứa `sequence` và `produced_at`. |
| `consumer` | Priority 1, stack 224 words | Receive forever, xử lý chậm 200 ticks. |
| `producer` | Priority 3, stack 224 words | Send timeout 100 ticks, tạo sequence liên tục. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Dung lượng queue | 2 |
| Timeout gửi | 100 ticks |
| Độ trễ consumer | 200 ticks |
| Timeout nhận | `HR_WAIT_FOREVER` |

### Target và khả năng port

Application sử dụng public kernel/framework API và `board.h`. CPU flags, startup, linker script, port, tick IRQ, fault backend, driver và OpenOCD được lấy từ `cmake/targets/<target>.cmake`. Các chi tiết LED, UART, clock hoặc marker trong README là hành vi của target tham chiếu `bluepill_f103c8`; target khác phải cung cấp board service tương đương.

## 4. Luồng thực thi

1. Consumer priority cao chạy trước và block vì queue rỗng.
2. Producer send item đầu; queue direct-handoff vào buffer của consumer.
3. Consumer READY và preempt producer.
4. Consumer in message rồi delay 200 ticks.
5. Producer tiếp tục fill queue; khi full, send block tối đa 100 ticks và có thể timeout.
6. Consumer thức dậy, lấy FIFO item và lặp lại.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hairtos/hr_queue.h`
- `hairtos/hr_time.h`

### API trọng tâm

- `hr_queue_create_static()`
- `hr_queue_send()`
- `hr_queue_receive()`
- `hr_queue_get_count()`

### Module được đưa vào bản biên dịch

- `task_kernel`
- `kernel_runtime`
- `kernel_time`
- `queue`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make TARGET=bluepill_f103c8 EXAMPLE=09-queue-blocking-ipc build` |
| Flash và chạy | `make TARGET=bluepill_f103c8 EXAMPLE=09-queue-blocking-ipc run` |
| Kiểm tra | `make TARGET=bluepill_f103c8 EXAMPLE=09-queue-blocking-ipc check` |
| Dọn build riêng | `make TARGET=bluepill_f103c8 EXAMPLE=09-queue-blocking-ipc clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TARGET=bluepill_f103c8 TOOLCHAIN=clang EXAMPLE=09-queue-blocking-ipc build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos queue and blocking IPC
Static FIFO queue with blocking send/receive and timeout.
consumer received seq=1 produced_at=0 now=0 queued=0 send_timeouts=0
consumer received seq=<tăng> produced_at=<tick> now=<tick> queued=<0..2> send_timeouts=<có thể tăng>
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Sequence nhận được luôn tăng, dù có thể có khoảng trống do send timeout.
- Consumer preempt producer khi được đánh thức.
- Queue count luôn trong 0..2 và không có blocking API error.

### Lỗi thường gặp

- Sequence giảm/lặp: lỗi FIFO hoặc direct handoff.
- Producer không timeout khi queue full: kiểm tra wait/timeout integration.
- Receiver thức nhưng không preempt: kiểm tra higher-priority wake flag.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=09-queue-blocking-ipc clean
make TARGET=bluepill_f103c8 EXAMPLE=09-queue-blocking-ipc build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Một producer và một consumer; nhiều waiter được unit-test ở host nhưng không trình diễn tại đây.

- Khi chạy trên target khác, pin, clock, CPU name, marker và output phần cứng lấy từ board/target manifest; không nên xem giá trị của Blue Pill là contract chung.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`10-01-semaphore-from-isr`](../10-01-semaphore-from-isr/README.md). Bài tiếp theo đồng bộ ISR → task bằng semaphore.

### Liên hệ Version 2

Queue direct-handoff và blocking contract là invariant cần giữ; Version 2 có thể tăng diagnostics chứ không đổi ownership âm thầm.

Xem [`../../docs/09-version2/README.md`](../../docs/09-version2/README.md).
