# Changelog

> Lịch sử thay đổi của `hairtos`. Version hiện tại trong source: `1.0.0-rc1`.

Format bám theo tinh thần *Keep a Changelog* và Semantic Versioning. Phần `Unreleased` chỉ mô tả thay đổi chưa gắn tag; `docs/09-version2` là roadmap và không làm thay đổi version runtime hiện tại.

## [Unreleased]

### Documentation

- Audit lại kernel, `haievent`, Cortex-M3 port, STM32F1 platform, driver, CMake/Make, examples, tests, diagnostics, benchmark và allocator lab.
- Viết lại toàn bộ Markdown first-party theo cấu trúc: scope → contract → source mapping → invariant → failure semantics → validation → references.
- Phân tách rõ v1 implementation (`docs/00`–`08`) với Version 2 proposal (`docs/09-version2`).
- Bổ sung navigation/TOC/Mermaid đúng semantic và validation baseline từ host test suite.

## [1.0.0-rc1] - 2026-07-27

### Added

- Static-first Cortex-M3 kernel: task, scheduler, timeout, queue, semaphore, mutex, software timer, diagnostics và retained fault record.
- `haievent`: Active Object, flat state machine, time event, fixed-block event pool và publish/subscribe.
- Host sanitizer tests, deterministic scheduler stress, allocator lab và target benchmark.
- Target manifest tách architecture/SoC/board/driver khỏi generic build graph.
- `VERSION` và changelog.

### Changed

- CMake trở thành source-of-truth cho target/example/module/source; Makefile trở thành command wrapper.
- `kernel/internal` và `haievent/internal` chỉ được cấp cho test/example/module có lý do rõ ràng.
- Driver public API và STM32F1 backend được tách riêng.
- Kernel tick IRQ được chuyển ra architecture layer.
- Benchmark clock/board marker được tách khỏi statistics generic.

### Removed

- Placeholder benchmark trùng lặp.
- Skeleton/tài liệu không còn tham gia build runtime.

## References

- [Semantic Versioning 2.0.0](https://semver.org/)
