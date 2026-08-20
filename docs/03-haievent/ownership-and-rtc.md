# Event ownership và run-to-completion

> **Phạm vi:** Implementation `hairtos 1.0.0-rc1`, bao gồm source, config, build graph và host-test evidence hiện có.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](event-model.md) · [Next →](publish-subscribe.md)

## Mục lục

- [Tổng quan và bản chất](#tong-quan)
- [Implementation trong repository](#implementation)
- [Mô hình và luồng thực thi](#mo-hinh)
- [Ownership, concurrency và invariants](#invariants)
- [Failure modes và giới hạn](#failure)
- [Validation và cách kiểm chứng](#validation)
- [Source map](#source-map)
- [Tài liệu tham khảo](#references)

<a id="tong-quan"></a>
## Tổng quan và bản chất

`haievent` phân biệt static event và dynamic event. Dynamic event sống trong fixed-block pool và dùng reference count; static event không được framework tự reclaim. Ownership contract là phần cốt lõi vì queue/AO/pub-sub có thể giữ cùng event qua nhiều consumer.


<a id="implementation"></a>
## Implementation trong repository

Implementation hiện tại gồm:

- Event pool là caller-owned arena chia block cố định; không dùng general heap.
- Dynamic event khởi tạo reference_count và chỉ quay về pool khi count giảm về 0.
- `he_active_post` retain trước khi enqueue; AO release sau dispatch; post thất bại phải rollback reference.
- Publish/subscribe snapshot subscriber list rồi post shared event; publish tiêu thụ reference động của publisher kể cả không subscriber nào nhận.
- Reference count là uint16_t và có overflow guard.
- tính toán ngắn;
- transition/post/publish;
- không `hr_task_delay`;
- không wait queue/semaphore forever;
- không giữ mutex lâu.


<a id="mo-hinh"></a>
## Mô hình và luồng thực thi

**Dynamic event lifetime**

```mermaid
stateDiagram-v2
    direction TB
    [*] --> FREE
    FREE --> OWNED: allocate
    OWNED --> SHARED: share
    SHARED --> OWNED: refs drop to one
    OWNED --> FREE: final release
```

Additional retain/release operations can change the reference count while the event remains in `SHARED`; they do not require a state transition.

**Static event ownership**

```mermaid
flowchart TB
    CALLER["Caller-owned storage"] --> POST["Post / dispatch"]
    POST --> SAME["Caller remains owner"]
```

Các function và source file tương ứng được liệt kê trong phần Source map.

<a id="invariants"></a>
## Ownership, concurrency và invariants

Các invariant nền áp dụng cho chủ đề này:

- Opaque object public chỉ hợp lệ sau create/init thành công và magic/internal state khớp contract.
- Intrusive node chỉ được linked vào đúng một list tại một thời điểm; remove/timeout/wake phải để node về trạng thái unlinked nhất quán.
- Thread API có thể block chỉ khi kernel RUNNING và không ở ISR; ISR API phải non-blocking và sử dụng `higher_priority_task_woken` khi cần defer switch sang PendSV.
- Critical section hiện dùng PRIMASK trên Cortex-M3, nghĩa là mask interrupt toàn cục trong đoạn ngắn; vì vậy code trong critical section phải bounded và không được gọi operation có thể block.
- Priority dùng **effective priority** ở ready/wait policy khi mutex inheritance đang active; base priority chỉ là cấu hình gốc.
- Static-first không có nghĩa “không có lifetime”: caller-owned TCB/stack/queue storage/event pool vẫn phải sống lâu hơn mọi object đang tham chiếu tới chúng.

<a id="failure"></a>
## Failure modes và giới hạn

- `hairtos 1.0.0-rc1` là single-core, không có SMP, FPU context, MPU isolation hay general dynamic kernel heap.
- Interrupt masking model hiện là PRIMASK; repository chưa có BASEPRI ceiling contract cho application ISR priority phức tạp.
- Tickless idle chưa có; time model hiện dựa trên tick 1 kHz ở target tham chiếu.
- `haievent` v1 là flat state machine và one-task-per-AO; HSM/deferred event/shared executor nằm ở roadmap Version 2.
- Build/link PASS không tự chứng minh real-time timing hoặc race-free behavior trên hardware; target tests và measurement vẫn cần thiết.

<a id="validation"></a>
## Validation và cách kiểm chứng

- Host suite của repository được build bằng GCC với AddressSanitizer + UndefinedBehaviorSanitizer và `ctest`.
- Host validation baseline: `make TARGET=bluepill_f103c8 host-tests` PASS.
- Host examples `02-kernel-data-structures-host`, `14-memory-allocator-lab`, `16-diagnostics-stress-stabilization` chạy PASS; stress scheduler report 500.000 iteration.
- Không suy ra target runtime PASS từ host test. Cortex-M3 assembly, timing, exception priority, UART/LED và hardware clock vẫn cần cross-build + board validation.


<a id="source-map"></a>
## Source map

- `haievent/src/he_event.c`
- `haievent/src/he_active.c`
- `haievent/src/he_pubsub.c`
- `tests/host/test_haievent.c`


<a id="references"></a>
## Tài liệu tham khảo


**Nguồn implementation trong repository:**
- `haievent/src/he_event.c`
- `haievent/src/he_active.c`
- `haievent/src/he_pubsub.c`
- `tests/host/test_haievent.c`
