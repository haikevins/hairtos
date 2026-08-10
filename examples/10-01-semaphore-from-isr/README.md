# `10-01-semaphore-from-isr` — Trao semaphore từ ISR

> **Môi trường:** Target. Target tham chiếu là `bluepill_f103c8`; target khác được chọn bằng `TARGET=<name>`.  
> **Vị trí mã nguồn:** `examples/10-01-semaphore-from-isr/main.c`  
> **Mục đích:** Software-triggered EXTI0 ISR give binary semaphore và đánh thức waiter priority cao sau khi ISR return.

## 1. Mục tiêu học tập

- Sử dụng API semaphore không blocking trong ISR.
- Truyền `higher_priority_task_woken` ra khỏi kernel API.
- Pend context switch sau ISR bằng `hr_yield_from_isr()`.
- Phân biệt ISR context với task context.

## 2. Kiến thức trọng tâm

- Binary semaphore là counting semaphore max count 1.
- ISR không được block hoặc dùng finite timeout.
- Context switch không xảy ra giữa ISR handler; PendSV chạy sau exception return.
- EXTI0 được trigger bằng SWIER nên không cần nút ngoài.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `waiter` | Priority 1, stack 192 words | Block trên binary semaphore. |
| `trigger` | Priority 3, stack 192 words | Mỗi 500 ticks ghi EXTI_SWIER. |
| EXTI0 IRQ | NVIC IRQ6 | Clear pending, give semaphore và request yield. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Semaphore initial | Unavailable |
| Trigger period | 500 ticks |
| External wiring | Không cần cho EXTI; chỉ UART/ST-Link |

### Target và khả năng port

Application sử dụng public kernel/framework API và `board.h`. CPU flags, startup, linker script, port, tick IRQ, fault backend, driver và OpenOCD được lấy từ `cmake/targets/<target>.cmake`. Các chi tiết LED, UART, clock hoặc marker trong README là hành vi của target tham chiếu `bluepill_f103c8`; target khác phải cung cấp board service tương đương.

## 4. Luồng thực thi

1. Waiter chạy trước và block trên semaphore.
2. Trigger task chạy, delay 500 ticks rồi software-trigger EXTI0.
3. ISR clear pending và gọi `hr_semaphore_give_from_isr()`.
4. Kernel đánh thức waiter và đặt cờ higher-priority.
5. `hr_yield_from_isr(true)` pend PendSV.
6. Sau ISR return, waiter preempt trigger và in wake counter.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hairtos/hr_semaphore.h`
- `hairtos/hr_context.h`
- `stm32f1.h`

### API trọng tâm

- `hr_semaphore_create_binary()`
- `hr_semaphore_take()`
- `hr_semaphore_give_from_isr()`
- `hr_yield_from_isr()`

### Module được đưa vào bản biên dịch

- `task_kernel`
- `kernel_runtime`
- `kernel_time`
- `semaphore`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make TARGET=bluepill_f103c8 EXAMPLE=10-01-semaphore-from-isr build` |
| Flash và chạy | `make TARGET=bluepill_f103c8 EXAMPLE=10-01-semaphore-from-isr run` |
| Kiểm tra | `make TARGET=bluepill_f103c8 EXAMPLE=10-01-semaphore-from-isr check` |
| Dọn build riêng | `make TARGET=bluepill_f103c8 EXAMPLE=10-01-semaphore-from-isr clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TARGET=bluepill_f103c8 TOOLCHAIN=clang EXAMPLE=10-01-semaphore-from-isr build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos semaphore from ISR
EXTI0 software interrupt gives a binary semaphore.
The higher-priority waiter preempts after ISR return.
waiter wake=1 irq_count=1 tick=500
waiter wake=2 irq_count=2 tick=1000
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Wake count và IRQ count cùng tăng.
- Waiter chạy ngay sau ISR return.
- Không có `semaphore ISR handoff failed`.

### Lỗi thường gặp

- IRQ không chạy: kiểm tra EXTI IMR, SWIER, PR và NVIC ISER.
- Semaphore full liên tục: waiter không consume hoặc ISR trigger quá nhanh.
- Không preempt: kiểm tra cờ `higher_priority_task_woken`.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=10-01-semaphore-from-isr clean
make TARGET=bluepill_f103c8 EXAMPLE=10-01-semaphore-from-isr build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Example truy cập thanh ghi EXTI trực tiếp; đây là demo ISR path, không phải driver EXTI tổng quát.

- Khi chạy trên target khác, pin, clock, CPU name, marker và output phần cứng lấy từ board/target manifest; không nên xem giá trị của Blue Pill là contract chung.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`10-02-mutex-priority-inheritance`](../10-02-mutex-priority-inheritance/README.md). Bài tiếp theo xử lý priority inversion bằng mutex inheritance.

### Liên hệ Version 2

Version 2 sẽ dùng bài ISR-wakeup này để kiểm tra max-syscall interrupt priority/BASEPRI contract trên Cortex-M target.

Xem [`../../docs/09-version2/README.md`](../../docs/09-version2/README.md).
