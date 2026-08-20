# 03 — haievent: Event-Driven Framework

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
| [`active-object.md`](active-object.md) | Active Object |
| [`architecture.md`](architecture.md) | Kiến trúc haievent |
| [`event-model.md`](event-model.md) | Event model |
| [`ownership-and-rtc.md`](ownership-and-rtc.md) | Event ownership và run-to-completion |
| [`publish-subscribe.md`](publish-subscribe.md) | Publish / Subscribe |
| [`state-machine.md`](state-machine.md) | Flat State Machine |
| [`time-event.md`](time-event.md) | Time Event |

<a id="validation"></a>
## Validation baseline

- `VERSION`: `1.0.0-rc1`.
- Host validation baseline: 64 test function trong suite hiện có đều PASS.
- `02-kernel-data-structures-host`, `14-memory-allocator-lab`, `16-diagnostics-stress-stabilization` chạy PASS trên host.
- Target tham chiếu là `bluepill_f103c8`; host evidence không thay thế cross-build, OpenOCD và hardware validation trên board.

<a id="references"></a>
## Tài liệu tham khảo


**Nguồn implementation trong repository:**
- `README.md`
- `CMakeLists.txt`
- `cmake/hairtos_examples.cmake`
- `cmake/hairtos_targets.cmake`
