# `05-cooperative-context-switch` — Chuyển ngữ cảnh hợp tác

> **Môi trường:** Target — STM32F103C8T6  
> **Vị trí mã nguồn:** `examples/05-cooperative-context-switch/main.c`  
> **Mục đích:** Hai task cùng priority chủ động nhường CPU bằng `hr_task_yield()`, còn PendSV thực hiện save/restore context.

## 1. Mục tiêu học tập

- Save PSP và R4–R11 của task đang chạy.
- Restore context của task kế tiếp.
- Xác nhận local variable và stack cookie của mỗi task được bảo toàn.
- Hiểu cooperative scheduling: task chỉ chuyển khi tự yield.

## 2. Kiến thức trọng tâm

- Hardware tự stack R0–R3, R12, LR, PC, xPSR.
- Port assembly stack/unstack R4–R11.
- TCB lưu saved PSP.
- Equal-priority FIFO được rotate khi yield.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `task-a` | Priority 2, stack 160 words | Counter bắt đầu từ 0 và tăng 1. |
| `task-b` | Priority 2, stack 160 words | Counter bắt đầu từ 1000 và tăng 10. |
| PendSV | Exception priority thấp | Điểm duy nhất đổi context. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Priority | Cả hai task = 2 |
| Khoảng in | Busy-wait 250 ms |
| Preemption | Không; chỉ yield |

## 4. Luồng thực thi

1. Task A bắt đầu qua SVC.
2. Task A kiểm tra PSP/current task, cập nhật local state và gọi `hr_task_yield()`.
3. Yield pend PendSV.
4. PendSV lưu context A, scheduler rotate queue priority 2, restore B.
5. Task B làm tương tự rồi yield trở lại A.
6. Chu kỳ lặp vô hạn, local counter không bị reset.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hairtos/hr_kernel.h`
- `hairtos/hr_task.h`
- `hr_port.h`

### API trọng tâm

- `hr_task_yield()`
- `hr_task_current()`
- `hr_kernel_start()`

### Module được đưa vào bản biên dịch

- `task_kernel`
- `kernel_runtime`
- `baremetal_tick`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make EXAMPLE=05-cooperative-context-switch build` |
| Flash và chạy | `make EXAMPLE=05-cooperative-context-switch run` |
| Kiểm tra | `make EXAMPLE=05-cooperative-context-switch check` |
| Dọn build riêng | `make EXAMPLE=05-cooperative-context-switch clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TOOLCHAIN=clang EXAMPLE=05-cooperative-context-switch build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos cooperative context switch
Two equal-priority tasks switch cooperatively through PendSV.
Starting task A through SVC; each yield pends PendSV.
task=A local_counter=1 -> yield
task=B local_counter=1010 -> yield
task=A local_counter=2 -> yield
task=B local_counter=1020 -> yield
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Dòng A/B xen kẽ ổn định.
- Counter A tăng 1; counter B tăng 10 và không bị reset.
- Không báo stack-local state corrupted.

### Lỗi thường gặp

- Chỉ một task chạy: kiểm tra PENDSVSET và selector.
- Counter bị hỏng: kiểm tra save/restore R4–R11, PSP alignment và TCB saved SP.
- HardFault khi exception return: kiểm tra stack frame và EXC_RETURN.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make EXAMPLE=05-cooperative-context-switch clean
make EXAMPLE=05-cooperative-context-switch build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Busy-wait trong task vẫn chiếm CPU.
- Không tự preempt task nếu task quên gọi yield.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`06-priority-scheduler`](../06-priority-scheduler/README.md). Bài tiếp theo áp dụng policy fixed-priority và FIFO giữa các task cùng priority.
