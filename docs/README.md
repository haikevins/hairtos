# Tài liệu hairtos

Bộ tài liệu này được viết dựa trên source hiện tại của `hairtos 1.0.0-rc1`. Mục tiêu là để một người mới có thể đi từ kiến trúc tổng thể xuống từng invariant/kernel path, còn người muốn port MCU có thể xác định chính xác phần nào phải giữ và phần nào phải thay.

> `docs/00`–`08` mô tả **v1 hiện tại**. `docs/09-version2` là **kế hoạch tương lai**, không phải capability đã có.

## Cách đọc

| Nhóm | Nội dung |
|---|---|
| [`00-overview`](00-overview/README.md) | Kiến trúc, audit source, capability, cấu hình, dependency |
| [`01-kernel-core`](01-kernel-core/README.md) | TCB, scheduler, context switch, tick, timeout, invariants |
| [`02-synchronization`](02-synchronization/README.md) | Queue, semaphore, mutex, blocking, timer, suspend/resume |
| [`03-haievent`](03-haievent/README.md) | Event ownership, FSM, Active Object, time event, pub/sub |
| [`04-platform`](04-platform/README.md) | Port contract, target manifest, SoC/board/driver, linker |
| [`05-api-reference`](05-api-reference/README.md) | Public API contract |
| [`06-testing-and-quality`](06-testing-and-quality/README.md) | Test, diagnostics, stress, benchmark, validation baseline |
| [`07-labs-and-examples`](07-labs-and-examples/README.md) | Example roadmap và allocator lab |
| [`08-appendices`](08-appendices/README.md) | Glossary và source map |
| [`09-version2`](09-version2/README.md) | Kế hoạch Version 2 |

## Nguồn sự thật

Khi tài liệu và source mâu thuẫn, source/header/CMake hiện hành là nguồn sự thật. Public API nằm trong `kernel/include/hairtos/` và `haievent/include/haievent/`; các file `internal/` không phải contract cho application.

## Quy ước

- Priority số nhỏ hơn = ưu tiên cao hơn.
- `host` = executable native; `target` = firmware cho MCU.
- `static-first` = caller cung cấp storage; kernel không phụ thuộc heap runtime.
- `AO` = Active Object.
- `RTC` = run-to-completion.
- `v2 planned` = chưa triển khai.
