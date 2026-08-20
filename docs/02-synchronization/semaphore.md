# Semaphore

> **Phạm vi:** Implementation `hairtos 1.0.0-rc1`, bao gồm source, config, build graph và host-test evidence hiện có.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](queue.md) · [Next →](software-timer.md)

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

Semaphore là synchronization counter không có ownership. Counting semaphore giữ `count/max_count`; binary semaphore chỉ là cấu hình max=1. `give` ưu tiên đánh thức waiter thay vì tăng count khi có task đang chờ.


<a id="implementation"></a>
## Implementation trong repository

Implementation hiện tại gồm:

- `take` tiêu thụ token nếu count > 0; nếu không có token thì có thể block theo timeout.
- `give` với waiter đang chờ chuyển quyền tiến triển trực tiếp sang waiter; nếu không có waiter mới tăng count.
- `give_from_isr` là ISR-safe và chỉ báo nhu cầu context switch qua output flag.
- Semaphore không theo dõi owner và không có priority inheritance; dùng mutex nếu cần mutual exclusion có ownership.
- Give khi count đã max và không có waiter trả `HR_ERROR_SEMAPHORE_FULL`.
- no-wait -> `HR_ERROR_SEMAPHORE_EMPTY`;
- finite/forever -> block trên waiter list.
- `count <= max_count`;
- max > 0;
- waiter ordering đúng;
- token không vừa increment vừa handoff cùng lần give.


<a id="mo-hinh"></a>
## Mô hình và luồng thực thi

```mermaid
flowchart TB
    TAKE["Take"] --> TOK{"count > 0?"}
    TOK -->|"Yes"| DEC["Decrement count"]
    TOK -->|"No + wait"| WAIT["Block task"]
    GIVE["Give"] --> W{"Waiter exists?"}
    W -->|"Yes"| WAKE["Wake highest-priority waiter"]
    W -->|"No"| INC["Increment count"]
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

- `kernel/src/hr_semaphore.c`
- `kernel/internal/hr_semaphore_internal.h`
- `tests/host/test_semaphore.c`


<a id="references"></a>
## Tài liệu tham khảo

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Nguồn implementation trong repository:**
- `kernel/src/hr_semaphore.c`
- `kernel/internal/hr_semaphore_internal.h`
- `tests/host/test_semaphore.c`
