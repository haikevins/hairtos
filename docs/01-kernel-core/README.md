# 01 — Lõi kernel

> **Vai trò:** Trang này là index của một cụm tài liệu. Mỗi chapter đi từ bản chất → mapping source → invariant → failure mode → validation để người đọc có thể đối chiếu trực tiếp với implementation.

[← Root README](../../README.md)

## Mục lục

- [Bản đồ nội dung](#ban-do)
- [Cách đọc](#cach-doc)
- [Các tài liệu](#tai-lieu)
- [Validation baseline](#validation)
- [Tài liệu tham khảo](#references)

<a id="ban-do"></a>
## Bản đồ nội dung

```mermaid
flowchart LR
    INDEX["Index"] --> CONCEPT["Concept / contract"]
    CONCEPT --> SOURCE["Source mapping"]
    SOURCE --> TEST["Validation / evidence"]
```

<a id="cach-doc"></a>
## Cách đọc

1. Bắt đầu từ README của section để biết scope và thứ tự học.
2. Khi gặp API, quay lại `docs/05-api-reference/` để xem context/return contract; khi gặp behavior kernel, ưu tiên `docs/01`–`03`.
3. Đối chiếu mọi statement timing/ownership với source map ở cuối chapter.
4. Phân biệt rõ **host evidence**, **target evidence** và **future proposal**.

<a id="tai-lieu"></a>
## Các tài liệu

| Tài liệu | Vai trò |
| --- | --- |
| [`context-switch.md`](context-switch.md) | Context switch |
| [`interrupt-model.md`](interrupt-model.md) | Interrupt model |
| [`intrusive-data-structures.md`](intrusive-data-structures.md) | Intrusive data structures |
| [`kernel-invariants.md`](kernel-invariants.md) | Kernel invariants |
| [`kernel-lifecycle.md`](kernel-lifecycle.md) | Kernel lifecycle |
| [`memory-model.md`](memory-model.md) | Memory model |
| [`scheduler.md`](scheduler.md) | Scheduler |
| [`task-model.md`](task-model.md) | Task model và TCB |
| [`time-and-timeout.md`](time-and-timeout.md) | Time và timeout |

<a id="validation"></a>
## Validation baseline

- `VERSION`: `1.0.0-rc1`.
- Host test suite hiện có 64 test function và đã chạy PASS trong lần audit tài liệu này.
- `02-kernel-data-structures-host`, `14-memory-allocator-lab`, `16-diagnostics-stress-stabilization` chạy PASS trên host.
- Target tham chiếu là `bluepill_f103c8`; cross toolchain/OpenOCD không có trong môi trường audit nên không tuyên bố đã flash lại hardware.

<a id="references"></a>
## Tài liệu tham khảo

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Nguồn implementation trong repository:**
- `README.md`
- `CMakeLists.txt`
- `cmake/hairtos_examples.cmake`
- `cmake/hairtos_targets.cmake`
