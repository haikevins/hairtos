# Nhật ký thay đổi

Tất cả thay đổi đáng chú ý của `hairtos` được ghi lại tại đây. Project sử dụng Semantic Versioning.

## [Chưa phát hành]

### Tài liệu

- Audit lại toàn bộ source v1: kernel, `haievent`, architecture port, SoC, board, driver, build system, tests, examples, diagnostics, benchmark và allocator lab.
- Mở rộng tài liệu kiến trúc, invariants, blocking contract, ownership, portability và validation baseline.
- Bổ sung `docs/00-overview/project-analysis.md` và `capability-matrix.md` để phân biệt rõ tính năng đã triển khai với giới hạn hiện tại.
- Bổ sung nhóm `docs/09-version2/` mô tả mục tiêu, kiến trúc, migration, test/release và roadmap cho Version 2.
- Version 2 chỉ là kế hoạch; version source hiện tại vẫn là `1.0.0-rc1`.

## [1.0.0-rc1] - 2026-07-27

### Bổ sung

- Kernel Cortex-M3 theo định hướng static-first với task, scheduler, timeout, queue, semaphore, mutex, software timer, diagnostics và retained fault record.
- Framework `haievent` với Active Object, flat state machine, time event, event pool và publish/subscribe.
- Host sanitizer tests, deterministic scheduler stress, allocator lab và target benchmark.
- Hệ thống target manifest để tách architecture/SoC/board/driver khỏi build root.
- `VERSION` và tài liệu changelog.

### Thay đổi

- CMake trở thành nguồn cấu hình duy nhất cho target/example/module/source.
- Makefile trở thành command wrapper thống nhất.
- `kernel/internal` và `haievent/internal` chỉ được cấp cho code cần internal contract.
- Driver API được tách thành public interface và implementation theo SoC.
- Kernel tick IRQ được chuyển ra architecture layer.
- Benchmark clock và board marker được tách khỏi benchmark statistics generic.

### Loại bỏ

- Placeholder benchmark trùng lặp.
- Tài liệu và skeleton không còn tham gia build.
