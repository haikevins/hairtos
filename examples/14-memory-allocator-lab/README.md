# `14-memory-allocator-lab` — Bài thực hành bộ cấp phát bộ nhớ

> **Môi trường:** Host hoặc Target. Target tham chiếu là `bluepill_f103c8`; target khác được chọn bằng `TARGET=<name>`.  
> **Vị trí mã nguồn:** `examples/14-memory-allocator-lab/main.c`  
> **Mục đích:** Lab độc lập gồm fixed-block pool và first-fit heap có split/coalesce; allocator không được kernel runtime sử dụng.

## 1. Mục tiêu học tập

- Hiểu fixed-size pool có allocation time xác định.
- Hiểu first-fit, block splitting và adjacent coalescing.
- Đo internal/external fragmentation.
- Phát hiện invalid pointer, double free và structural corruption qua validation/tests.

## 2. Kiến thức trọng tâm

- Vùng nhớ tĩnh do ứng dụng sở hữu.
- Alignment theo `max_align_t`.
- Heap block header và payload.
- Tái sử dụng theo chiến lược first-fit.
- Allocator lab tách khỏi TCB/queue/timer/AO.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Target heap arena | 2048 bytes | Chạy chuỗi alloc/free/coalesce và in stats qua UART. |
| Target pool arena | 512 bytes, 8 block payload 24 bytes | Cấp/free một block và validate. |
| Host demo | Stack arenas 2048/512 bytes | In stats bằng `printf`. |
| Host tests | ASan/UBSan | Kiểm tra edge cases và randomized workload. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Kernel dependency | Không |
| Vòng lặp target sau PASS | LED toggle 500 ms |
| Environment selection | Phải chỉ rõ host/target khi cần |

### Target và khả năng port

Application sử dụng public kernel/framework API và `board.h`. CPU flags, startup, linker script, port, tick IRQ, fault backend, driver và OpenOCD được lấy từ `cmake/targets/<target>.cmake`. Các chi tiết LED, UART, clock hoặc marker trong README là hành vi của target tham chiếu `bluepill_f103c8`; target khác phải cung cấp board service tương đương.

## 4. Luồng thực thi

1. Khởi tạo heap arena.
2. Cấp ba block và in stats.
3. Free middle rồi first để tạo/coalesce vùng trống.
4. Cấp block lớn 280 bytes và xác nhận first-fit trả về địa chỉ first.
5. Free tất cả và validate full coalescing.
6. Khởi tạo pool, alloc/free một block và validate.
7. Target in PASS; host process trả về exit code.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hr_heap_lab.h`
- `hr_pool_lab.h`

### API trọng tâm

- `hr_heap_lab_init()`
- `hr_heap_lab_alloc()`
- `hr_heap_lab_free()`
- `hr_heap_lab_get_stats()`
- `hr_heap_lab_validate()`
- `hr_pool_lab_*()`

### Module được đưa vào bản biên dịch

- `allocator` trên target
- `labs/memory-allocator/demo.c` trên host

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch trên host | `make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab build` |
| Chạy trên host | `make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run` |
| Biên dịch target | `make TARGET=bluepill_f103c8 ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab build` |
| Flash/chạy target | `make TARGET=bluepill_f103c8 ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab run` |
| Kiểm tra | `make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab check` |
| Dọn build | `make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab clean` |

Biến thể target có thể cross-build bằng Clang/LLD:

```bash
make TARGET=bluepill_f103c8 TOOLCHAIN=clang ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
# Host
after allocation: allocated=<...> free=<...> largest=<...> internal=<...> external=<...>
after middle free: ...
after coalescing: ...
pool: first=<ptr> second=<ptr> valid=yes

# Target
hairtos memory allocator lab
after alloc requested=<...> ...
after full coalesce ... external_frag=0
pool capacity=8 stride=<...> free=8
Memory allocator lab: PASS
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Chương trình host kết thúc với mã 0.
- Heap validate đúng sau full coalesce.
- External fragmentation về 0 sau free tất cả.
- Pool capacity/free count hợp lệ.
- Target in PASS và LED nháy.

### Lỗi thường gặp

- Large alloc không reuse first block: split/coalesce/first-fit lỗi.
- Validate fail: block chain/header bị hỏng.
- Double free hoặc invalid pointer phải trả status lỗi, không crash.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=14-memory-allocator-lab clean
make TARGET=bluepill_f103c8 EXAMPLE=14-memory-allocator-lab build
```

## 9. Giới hạn của ví dụ

- Allocator chỉ phục vụ học tập; không thread-safe và không tích hợp kernel.
- Không gọi allocator trong ISR hoặc scheduler hot path.
- Biên dịch target không thay thế kiểm thử sanitizer trên host.

- Khi chạy trên target khác, pin, clock, CPU name, marker và output phần cứng lấy từ board/target manifest; không nên xem giá trị của Blue Pill là contract chung.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`15-kernel-benchmark`](../15-kernel-benchmark/README.md). Bài tiếp theo đo latency và footprint của kernel trên Cortex-M3.

### Liên hệ Version 2

Allocator vẫn nên ở ngoài kernel core Version 2; nếu có dynamic convenience layer thì phải optional và caller-controlled.

Xem [`../../docs/09-version2/README.md`](../../docs/09-version2/README.md).
