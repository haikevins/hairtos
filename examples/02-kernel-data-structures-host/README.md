# `02-kernel-data-structures-host` — Cấu trúc dữ liệu kernel — Demo trên host

> **Môi trường:** Host — máy phát triển Linux/macOS  
> **Vị trí mã nguồn:** `examples/02-kernel-data-structures-host/main.c`  
> **Mục đích:** Minh họa ready set và wait list bằng các node intrusive mà chưa cần tạo task thật hoặc chạy trên Cortex-M3.

## 1. Mục tiêu học tập

- Hiểu priority 0 là mức ưu tiên cao nhất.
- Quan sát ready queue FIFO giữa các node cùng priority.
- Quan sát wait list được sắp xếp theo priority và giữ FIFO khi bằng nhau.
- Kiểm tra structural invariants bằng hàm validate.

## 2. Kiến thức trọng tâm

- Danh sách liên kết đôi intrusive.
- Ready bitmap và một FIFO queue cho mỗi priority.
- Owner pointer từ node trở về đối tượng chứa node.
- Host-native test không có ISR, task stack hoặc context switch.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| `communication` | Priority 1 | Phải được chọn trước hai sensor. |
| `sensor-a` | Priority 3 | Đứng trước `sensor-b` theo FIFO ban đầu. |
| `sensor-b` | Priority 3 | Lên đầu sau khi rotate queue priority 3. |
| Ready set | `hr_ready_set_t` | Chọn highest priority và rotate FIFO. |
| Wait list | `hr_wait_list_t` | Sắp waiter theo priority. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Môi trường | Native host compiler |
| Phần cứng | Không cần |
| Internal API | Có chủ đích để học cấu trúc kernel |

## 4. Luồng thực thi

1. Khởi tạo ready set và wait list.
2. Khởi tạo ba demo node và chèn vào ready set.
3. Xem phần tử ưu tiên cao nhất: `communication` priority 1.
4. Loại `communication`, peek `sensor-a`, rotate và peek `sensor-b`.
5. Chèn waiter theo thứ tự không ưu tiên và xác nhận `communication` vẫn đứng đầu.
6. Validate cả hai cấu trúc trước khi trả về `EXIT_SUCCESS`.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hr_scheduler_internal.h`
- `hr_wait_internal.h`

### API trọng tâm

- `hr_ready_set_init()`
- `hr_ready_set_insert()`
- `hr_ready_set_peek_highest()`
- `hr_ready_set_rotate_highest()`
- `hr_wait_list_insert()`
- `hr_*_validate()`

### Module được đưa vào bản biên dịch

- `kernel/src/hr_list.c`
- `kernel/src/hr_scheduler.c`
- `kernel/src/hr_wait.c`

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make EXAMPLE=02-kernel-data-structures-host build` |
| Chạy | `make EXAMPLE=02-kernel-data-structures-host run` |
| Kiểm tra | `make EXAMPLE=02-kernel-data-structures-host check` |
| Dọn build | `make EXAMPLE=02-kernel-data-structures-host clean` |

Host example dùng compiler native do CMake phát hiện. Có thể đặt `CC=clang` hoặc cấu hình CMake host riêng khi cần so sánh compiler.

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
highest-ready: communication
equal-priority-before-rotate: sensor-a
equal-priority-after-rotate: sensor-b
first-waiter: communication
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Process trả về exit code 0.
- Bốn dòng output đúng thứ tự.
- Ready set và wait list đều validate thành công.

### Lỗi thường gặp

- Sai thứ tự priority: kiểm tra quy ước số nhỏ hơn là ưu tiên cao hơn.
- Rotate không đổi node đầu: kiểm tra FIFO queue có ít nhất hai node cùng priority.
- Sanitizer báo lỗi: kiểm tra double insertion/removal và owner pointer.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make EXAMPLE=02-kernel-data-structures-host clean
make EXAMPLE=02-kernel-data-structures-host build
```

## 9. Giới hạn của ví dụ

- Không tạo TCB thật.
- Không mô phỏng Cortex-M3 exception frame.
- Không chứng minh concurrency hoặc interrupt safety.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`03-static-task-stack`](../03-static-task-stack/README.md). Bài tiếp theo tạo TCB và initial task stack thật cho Cortex-M3.
