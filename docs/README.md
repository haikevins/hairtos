# Tài liệu hairtos

Tài liệu này mô tả **hairtos mainline** trên STM32F103C8T6/Cortex-M3. Đây là tài liệu của sản phẩm hiện hành, không phải gói tài liệu riêng cho một phase. Bộ tài liệu được tổ chức theo subsystem và chỉ giữ những nội dung cần để hiểu kiến trúc, sử dụng API, kiểm thử và bảo trì project.

## Cách đọc đề xuất

1. Đọc `00-overview/` để nắm kiến trúc, layout và cấu hình.
2. Đọc `01-kernel-core/` theo thứ tự từ lifecycle đến interrupt model.
3. Đọc `02-synchronization/` khi sử dụng queue, semaphore, mutex hoặc timer.
4. Đọc `03-haievent/` nếu xây dựng ứng dụng event-driven.
5. Dùng `05-api-reference/` như tài liệu tra cứu khi viết code.
6. Dùng `06-testing-and-quality/` trước khi thay đổi kernel hoặc phát hành.
7. Dùng `07-labs-and-examples/` để chọn đúng bài host hoặc target.

## Cấu trúc

| Thư mục | Nội dung |
|---|---|
| `00-overview/` | Kiến trúc, layout, cấu hình, coding standard và roadmap |
| `01-kernel-core/` | TCB, scheduler, SVC/PendSV, SysTick, timeout và interrupt |
| `02-synchronization/` | Queue, semaphore, mutex, suspend/resume và software timer |
| `03-haievent/` | Event model, state machine, Active Object, time event và pub/sub |
| `04-platform/` | Cortex-M3 port, STM32F1, startup, linker, driver và porting |
| `05-api-reference/` | API public của hairtos và haievent |
| `06-testing-and-quality/` | Testing, diagnostics, stress, benchmark, hygiene và release |
| `07-labs-and-examples/` | Danh mục example, allocator lab và cách chạy host/target |
| `08-appendices/` | Glossary và source map |

## Quy ước

- Tên API, macro, register và symbol giữ nguyên tiếng Anh.
- Priority số nhỏ hơn có mức ưu tiên cao hơn.
- `Host` là chương trình chạy bằng compiler native trên máy phát triển.
- `Target` là firmware cross-compile cho STM32F103/Cortex-M3.
- Build target chỉ xác nhận compile/link/ABI; runtime cần flash lên phần cứng.
- Số liệu benchmark chỉ so sánh được khi giữ cùng board, clock, compiler và cấu hình.

## Tài liệu đã loại khỏi bản rút gọn

- Nhật ký riêng cho từng Phase 0–16: roadmap và source hiện hành đã thay thế.
- Tài liệu migration từ cấu trúc docs cũ.
- Cortex-M0 compile proof và requirements coverage: chỉ là phụ lục tùy chọn.
- Tài liệu benchmark lab riêng: đã được gộp vào kernel benchmark và example index.
- Tài liệu host-vs-target riêng: đã được gộp vào example index.
