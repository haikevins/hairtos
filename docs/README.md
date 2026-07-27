# Tài liệu hairtos

## 1. Mục đích

Thư mục `docs/` mô tả kiến trúc, contract, API, kiểm thử, example và quy trình port của `hairtos`. Tài liệu được tổ chức theo dependency từ tổng quan đến platform-specific implementation.

## 2. Cách đọc đề xuất

1. [`00-overview/`](00-overview/README.md): mục tiêu, dependency, cấu hình và roadmap.
2. [`01-kernel-core/`](01-kernel-core/README.md): task, scheduler, context, time và memory model.
3. [`02-synchronization/`](02-synchronization/README.md): queue, semaphore, mutex, timer và suspend/resume.
4. [`03-haievent/`](03-haievent/README.md): event, state machine, Active Object và publish/subscribe.
5. [`04-platform/`](04-platform/README.md): architecture port, SoC, board, driver, startup/linker và target manifest.
6. [`05-api-reference/`](05-api-reference/README.md): public API.
7. [`06-testing-and-quality/`](06-testing-and-quality/README.md): test, diagnostics, stress và benchmark.
8. [`07-labs-and-examples/`](07-labs-and-examples/README.md): lộ trình thực hành.
9. [`08-appendices/`](08-appendices/README.md): glossary và source map.

## 3. Quy ước kiến trúc

- `kernel/` và `haievent/` là phần generic.
- `arch/` chứa CPU/ISA exception và context contract.
- `soc/` chứa register, startup và clock/IRQ support.
- `boards/` chứa board service, pin mapping và linker script.
- `drivers/` chia public API và implementation theo SoC.
- `cmake/targets/<target>.cmake` là binding duy nhất giữa build và phần cứng.

## 4. Quy ước tài liệu

- API, macro, file path và command được giữ nguyên bằng tiếng Anh kỹ thuật.
- Priority 0 là mức ưu tiên cao nhất.
- Tick mặc định là 1 kHz ở target tham chiếu nhưng backend do target quyết định.
- Build thành công không đồng nghĩa runtime đã được xác nhận trên hardware.
- Các ví dụ target mặc định dùng `TARGET=bluepill_f103c8`.

## 5. Tài liệu port quan trọng

- [`04-platform/porting-new-target.md`](04-platform/porting-new-target.md)
- [`04-platform/cortex-m3-port.md`](04-platform/cortex-m3-port.md)
- [`04-platform/startup-and-linker.md`](04-platform/startup-and-linker.md)
- [`04-platform/drivers.md`](04-platform/drivers.md)
- [`00-overview/dependency-rules.md`](00-overview/dependency-rules.md)

## 6. Kiểm tra tài liệu

Khi đổi target, API hoặc module mapping, cần cập nhật README tương ứng, tài liệu platform/API và example liên quan. Relative Markdown links phải tiếp tục phân giải được trong repository.
