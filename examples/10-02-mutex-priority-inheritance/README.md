# `10-02-mutex-priority-inheritance` — Mutex và kế thừa ưu tiên

> **Môi trường:** Target. Target tham chiếu là `bluepill_f103c8`; target khác được chọn bằng `TARGET=<name>`.  
> **Vị trí mã nguồn:** `examples/10-02-mutex-priority-inheritance/main.c`  
> **Mục đích:** Tái tạo priority inversion High–Medium–Low và chứng minh mutex boost effective priority của owner thấp.

## 1. Mục tiêu học tập

- Phân biệt base priority và effective priority.
- Quan sát task high block trên mutex do low sở hữu.
- Ngăn medium task làm low bị starvation bằng priority inheritance.
- Xác nhận direct ownership handoff và priority restoration.

## 2. Kiến thức trọng tâm

- Quyền sở hữu mutex.
- Priority inheritance từ waiter cao nhất.
- Owner được requeue khi effective priority thay đổi.
- Unlock chuyển ownership trực tiếp cho waiter phù hợp.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| `high` | Priority 1, stack 224 words | Thức ở tick 10 và chờ mutex. |
| `medium` | Priority 3, stack 224 words | Thức ở tick 20, CPU-bound cho đến PASS. |
| `low` | Priority 5, stack 224 words | Lock mutex trước và làm việc đến tick 120. |
| Mutex | Non-recursive | Bảo vệ resource giả lập. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Thời điểm đánh thức High | Tick 10 |
| Thời điểm đánh thức Medium | Tick 20 |
| Deadline công việc của Low | Tick 120 |

### Target và khả năng port

Application sử dụng public kernel/framework API và `board.h`. CPU flags, startup, linker script, port, tick IRQ, fault backend, driver và OpenOCD được lấy từ `cmake/targets/<target>.cmake`. Các chi tiết LED, UART, clock hoặc marker trong README là hành vi của target tham chiếu `bluepill_f103c8`; target khác phải cung cấp board service tương đương.

## 4. Luồng thực thi

1. Low chạy đầu tiên và lock mutex.
2. High thức ở tick 10, lock thất bại và block.
3. Kernel boost effective priority của Low từ 5 lên 1.
4. Medium thức ở tick 20 nhưng không thể preempt Low đã được boost.
5. Low hoàn tất và unlock; ownership chuyển cho High.
6. Low trở lại priority 5; High chạy và in PASS.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hairtos/hr_mutex.h`
- `hairtos/hr_time.h`

### API trọng tâm

- `hr_mutex_create()`
- `hr_mutex_lock()`
- `hr_mutex_unlock()`
- `hr_mutex_get_owner()`
- `hr_mutex_get_waiting_tasks()`
- `hr_task_get_effective_priority()`

### Module được đưa vào bản biên dịch

- `task_kernel`
- `kernel_runtime`
- `kernel_time`
- `mutex`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make TARGET=bluepill_f103c8 EXAMPLE=10-02-mutex-priority-inheritance build` |
| Flash và chạy | `make TARGET=bluepill_f103c8 EXAMPLE=10-02-mutex-priority-inheritance run` |
| Kiểm tra | `make TARGET=bluepill_f103c8 EXAMPLE=10-02-mutex-priority-inheritance check` |
| Dọn build riêng | `make TARGET=bluepill_f103c8 EXAMPLE=10-02-mutex-priority-inheritance clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TARGET=bluepill_f103c8 TOOLCHAIN=clang EXAMPLE=10-02-mutex-priority-inheritance build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos mutex priority inheritance
Priority inversion corrected by mutex inheritance.
High=1, Medium=3, Low=5.
low: owns mutex at base priority 5
high: attempts mutex and blocks
low: inherited priority=1; unlocking
high: acquired at tick=<...> medium_work=<...> PASS
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Low effective priority trở thành 1 trong lúc High chờ.
- High nhận ownership sau unlock.
- Sau handoff, Low được restore về base priority 5.
- Medium không ngăn completion của critical section.

### Lỗi thường gặp

- Low không inherit: kiểm tra waiter priority propagation và ready requeue.
- High không nhận owner: kiểm tra direct handoff.
- Low không restore: kiểm tra recompute effective priority từ owned mutex list.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=10-02-mutex-priority-inheritance clean
make TARGET=bluepill_f103c8 EXAMPLE=10-02-mutex-priority-inheritance build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Deadlock detection không thuộc example.
- Chained inheritance có unit test nhưng scenario target này chỉ dùng một mutex.

- Khi chạy trên target khác, pin, clock, CPU name, marker và output phần cứng lấy từ board/target manifest; không nên xem giá trị của Blue Pill là contract chung.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`11-task-suspend-resume`](../11-task-suspend-resume/README.md). Bài tiếp theo thêm administrative suspend/resume cho task ở nhiều trạng thái.

### Liên hệ Version 2

Version 2 ưu tiên deadlock diagnostics/trace, nhưng chained priority inheritance hiện tại vẫn là behavior nền.

Xem [`../../docs/09-version2/README.md`](../../docs/09-version2/README.md).
