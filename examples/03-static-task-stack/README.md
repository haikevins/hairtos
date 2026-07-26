# `03-static-task-stack` — TCB tĩnh và ngăn xếp khởi tạo của tác vụ

> **Môi trường:** Target — STM32F103C8T6  
> **Vị trí mã nguồn:** `examples/03-static-task-stack/main.c`  
> **Mục đích:** Tạo một task hoàn toàn tĩnh và dựng initial exception frame Cortex-M3, nhưng chưa khởi chạy scheduler.

## 1. Mục tiêu học tập

- Tạo opaque `hr_task_t` bằng API public.
- Cấp stack tĩnh từ application.
- Đưa task vào trạng thái CREATED và kiểm tra frame khởi tạo.
- Phân biệt tạo task với đăng ký/start task.

## 2. Kiến thức trọng tâm

- TCB static-first.
- Stack fill và stack guard.
- Initial frame chứa R0–R3, R12, LR, PC, xPSR và vùng R4–R11.
- Task argument được đặt vào R0 để dùng khi task bắt đầu.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| Task `demo` | Priority 2, stack 96 words | Nhận con trỏ `counter` nhưng không được thực thi trong bài này. |
| TCB | `g_demo_task` | Opaque public storage. |
| Stack | `g_demo_stack[96]` | Vùng RAM tĩnh cho initial frame và runtime stack. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Task state sau create | `HR_TASK_STATE_CREATED` |
| Kernel start | Không gọi |
| Delay LED | 500 ms bare-metal |

## 4. Luồng thực thi

1. Khởi tạo board.
2. Gọi `hr_task_create_static()` với entry, argument, stack và priority.
3. Kernel task layer fill stack, đặt guard và dựng initial frame.
4. In trạng thái tạo thành công.
5. Main tiếp tục chạy bare-metal và nháy LED; `demo_task()` không được gọi.

## 5. API và mã nguồn liên quan

### Header được dùng

- `board.h`
- `hairtos/hr_task.h`

### API trọng tâm

- `hr_task_create_static()`
- `board_delay_ms()`

### Module được đưa vào bản biên dịch

- `platform`
- `baremetal_tick`
- `task_kernel`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make EXAMPLE=03-static-task-stack build` |
| Flash và chạy | `make EXAMPLE=03-static-task-stack run` |
| Kiểm tra | `make EXAMPLE=03-static-task-stack check` |
| Dọn build riêng | `make EXAMPLE=03-static-task-stack clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TOOLCHAIN=clang EXAMPLE=03-static-task-stack build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos static task stack
Creating a static TCB and Cortex-M3 initial stack frame...
Task object: CREATED
Initial stack frame: READY
Task execution starts in the SVC startup example.
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Thông báo tạo task thành công xuất hiện.
- Main vẫn nháy LED; không có dấu hiệu `demo_task()` đã chạy.
- Build map cho thấy TCB và stack nằm trong RAM tĩnh.

### Lỗi thường gặp

- `Task creation failed`: kiểm tra stack pointer, số word tối thiểu, priority và alignment.
- HardFault trước log: kiểm tra startup/linker và stack main.
- Nếu counter tăng thì code đã vô tình start task, không còn đúng phạm vi bài.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make EXAMPLE=03-static-task-stack clean
make EXAMPLE=03-static-task-stack build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Chưa có `hr_kernel_init()`, idle task, SVC hoặc PSP Thread mode.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`04-start-first-task`](../04-start-first-task/README.md). Bài tiếp theo đăng ký task và khởi chạy task đầu tiên bằng SVC.
