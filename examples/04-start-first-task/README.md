# `04-start-first-task` — Khởi chạy tác vụ đầu tiên bằng SVC

> **Môi trường:** Target. Target tham chiếu là `bluepill_f103c8`; target khác được chọn bằng `TARGET=<name>`.  
> **Vị trí mã nguồn:** `examples/04-start-first-task/main.c`  
> **Mục đích:** Chuyển từ `main()` chạy bằng MSP sang task đầu tiên chạy ở Thread mode bằng PSP thông qua SVC.

## 1. Mục tiêu học tập

- Khởi tạo kernel và idle task.
- Đăng ký một application task vào ready set.
- Khởi chạy scheduler bằng `hr_kernel_start()`.
- Xác nhận argument được restore qua R0 và Thread mode dùng PSP.

## 2. Kiến thức trọng tâm

- SVC là exception chuyển quyền từ startup code sang kernel port.
- MSP dùng cho exception/handler; PSP dùng cho task Thread mode.
- Exception return `0xFFFFFFFD` khôi phục hardware frame từ PSP.
- `main()` không được quay lại sau khi kernel start thành công.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| Task `first-task` | Priority 2, stack 128 words | Task application đầu tiên. |
| Idle task | Priority thấp nhất, tạo nội bộ | Fallback khi không có task application READY. |
| Argument | Magic `0x50483421` | Kiểm tra R0 được restore đúng. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Context switch nhiều task | Chưa có |
| Cơ chế khởi chạy | SVC |
| Ngăn xếp Thread mode | PSP |

### Target và khả năng port

Application sử dụng public kernel/framework API và `board.h`. CPU flags, startup, linker script, port, tick IRQ, fault backend, driver và OpenOCD được lấy từ `cmake/targets/<target>.cmake`. Các chi tiết LED, UART, clock hoặc marker trong README là hành vi của target tham chiếu `bluepill_f103c8`; target khác phải cung cấp board service tương đương.

## 4. Luồng thực thi

1. `hr_kernel_init()` tạo scheduler và idle task.
2. Tạo `first-task`, sau đó `hr_task_start()` đưa task vào READY.
3. `hr_kernel_start()` chọn task ưu tiên cao nhất và gọi port start.
4. SVC handler restore R4–R11, nạp PSP và exception-return.
5. `first_task()` xác nhận current task, PSP và argument.
6. Task in heartbeat vô hạn; `main()` không tiếp tục.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hairtos/hr_kernel.h`
- `hairtos/hr_task.h`
- `hr_port.h`

### API trọng tâm

- `hr_kernel_init()`
- `hr_task_create_static()`
- `hr_task_start()`
- `hr_kernel_start()`
- `hr_task_current()`
- `hr_task_get_name()`

### Module được đưa vào bản biên dịch

- `task_kernel`
- `kernel_runtime`
- `baremetal_tick`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make TARGET=bluepill_f103c8 EXAMPLE=04-start-first-task build` |
| Flash và chạy | `make TARGET=bluepill_f103c8 EXAMPLE=04-start-first-task run` |
| Kiểm tra | `make TARGET=bluepill_f103c8 EXAMPLE=04-start-first-task check` |
| Dọn build riêng | `make TARGET=bluepill_f103c8 EXAMPLE=04-start-first-task clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TARGET=bluepill_f103c8 TOOLCHAIN=clang EXAMPLE=04-start-first-task build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos first-task startup
Preparing idle task and first application task...
Invoking SVC to leave main/MSP and enter task/PSP...
First task entered through SVC.
Current task: first-task
PSP active: yes
Task argument: valid
First-task startup: PASS
first-task heartbeat=1
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Có dòng `First-task startup: PASS`.
- `Current task` đúng là `first-task`.
- Không xuất hiện dòng `ERROR: hr_kernel_start returned`.

### Lỗi thường gặp

- PSP không active: kiểm tra CONTROL.SPSEL, SVC handler và EXC_RETURN.
- Argument sai: kiểm tra layout initial frame và offset R0.
- `hr_kernel_start()` quay lại: lỗi startup hoặc SVC path.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=04-start-first-task clean
make TARGET=bluepill_f103c8 EXAMPLE=04-start-first-task build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Chỉ một application task; chưa save context hiện tại để chuyển sang task khác.

- Khi chạy trên target khác, pin, clock, CPU name, marker và output phần cứng lấy từ board/target manifest; không nên xem giá trị của Blue Pill là contract chung.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`05-cooperative-context-switch`](../05-cooperative-context-switch/README.md). Bài tiếp theo bổ sung PendSV để chuyển qua lại giữa hai task.

### Liên hệ Version 2

Version 2 không bắt buộc SVC trên mọi architecture; example nên kiểm chứng semantic “first task starts correctly” qua port contract.

Xem [`../../docs/09-version2/README.md`](../../docs/09-version2/README.md).
