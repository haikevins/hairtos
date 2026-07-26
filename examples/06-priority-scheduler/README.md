# `06-priority-scheduler` — Bộ lập lịch ưu tiên cố định

> **Môi trường:** Target — STM32F103C8T6  
> **Vị trí mã nguồn:** `examples/06-priority-scheduler/main.c`  
> **Mục đích:** Kiểm chứng scheduler luôn chọn priority cao nhất, không phụ thuộc thứ tự đăng ký, và chỉ round-robin trong cùng priority khi yield.

## 1. Mục tiêu học tập

- Hiểu quy ước priority số nhỏ hơn là khẩn cấp hơn.
- Phân biệt registration order với scheduling order.
- Kiểm tra FIFO giữa `high-a` và `high-b`.
- Chứng minh task low không được chạy khi high tasks luôn READY.

## 2. Kiến thức trọng tâm

- Ready queues theo priority.
- Ready bitmap tìm priority cao nhất.
- Yield rotate queue hiện tại, không hạ xuống priority thấp hơn khi vẫn còn task high READY.
- Scheduler policy vẫn cooperative ở thời điểm này.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `high-a` | Priority 1, stack 160 words | Yield cho peer cùng priority. |
| `high-b` | Priority 1, stack 160 words | Yield cho peer cùng priority. |
| `low` | Priority 5, stack 160 words | Đăng ký trước nhưng phải không được chạy. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Registration order | low → high-a → high-b |
| Expected run order | high-a ↔ high-b |
| Preemption runtime | Chưa bật |

## 4. Luồng thực thi

1. Tạo và đăng ký low trước.
2. Đăng ký hai high task sau.
3. Scheduler chọn `high-a` vì priority 1.
4. `high-a` yield làm FIFO priority 1 chuyển `high-b` lên đầu.
5. `high-b` yield quay lại `high-a`.
6. Nếu `low` chạy khi high còn READY, example panic.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hairtos/hr_kernel.h`
- `hairtos/hr_task.h`

### API trọng tâm

- `hr_task_get_effective_priority()`
- `hr_task_yield()`
- `hr_task_start()`

### Module được đưa vào bản biên dịch

- `task_kernel`
- `kernel_runtime`
- `baremetal_tick`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make EXAMPLE=06-priority-scheduler build` |
| Flash và chạy | `make EXAMPLE=06-priority-scheduler run` |
| Kiểm tra | `make EXAMPLE=06-priority-scheduler check` |
| Dọn build riêng | `make EXAMPLE=06-priority-scheduler clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TOOLCHAIN=clang EXAMPLE=06-priority-scheduler build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos priority scheduler
Fixed-priority scheduler: smaller number means higher priority.
Low task is registered first but must never run while high tasks are READY.
selected=high-A priority=1 counter=1 -> yield to equal-priority peer
selected=high-B priority=1 counter=1010 -> yield to equal-priority peer
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Chỉ thấy `high-A` và `high-B` xen kẽ.
- Không thấy lỗi low-priority task ran.
- Effective priority của hai high task vẫn là 1.

### Lỗi thường gặp

- Low chạy: kiểm tra ready bitmap/highest selection.
- Hai high không FIFO: kiểm tra rotate queue.
- Scheduler chọn theo registration order: ready set chưa dùng priority đúng.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make EXAMPLE=06-priority-scheduler clean
make EXAMPLE=06-priority-scheduler build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Task priority cao mới READY chưa tự preempt task đang chạy.
- Chưa có blocking delay của kernel.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`07-task-delay-timeout`](../07-task-delay-timeout/README.md). Bài tiếp theo giao SysTick cho kernel và cho phép task block theo timeout.
