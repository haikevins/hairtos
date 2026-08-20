# Mutex và priority inheritance

> **Phạm vi:** Mô tả implementation `hairtos 1.0.0-rc1` đã được đối chiếu với source, config, build graph và host tests hiện có.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](blocking-contract.md) · [Next →](queue.md)

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

Mutex có ownership, recursion policy và priority inheritance. Owner giữ mutex trong `owned_mutexes`; effective priority được tính lại từ base priority và waiter cao nhất của toàn bộ mutex đang sở hữu. Recompute có thể lan theo chain khi owner lại đang block trên mutex khác.

Trong project này, cách đọc đúng luôn là **contract → data ownership → state transition → concurrency boundary → failure semantics → evidence**. Điều đó quan trọng hơn việc chỉ nhớ tên API: một RTOS nhỏ vẫn có thể sai nghiêm trọng nếu cùng một task node xuất hiện ở hai list, nếu timeout và object wake cùng “thắng”, hoặc nếu context switch không khớp exception frame của CPU.

<a id="implementation"></a>
## Implementation trong repository

Các điểm đã được đối chiếu với source/config hiện tại:

- Non-recursive mutex từ chối lock lại bởi chính owner; recursive mutex tăng recursion count và chỉ release ownership khi count về 0.
- Waiter priority cao hơn có thể boost owner; task READY phải được requeue theo effective priority mới.
- Chained inheritance được hỗ trợ bằng recompute đệ quy có bound phòng cycle/pathological depth.
- Unlock chỉ hợp lệ với owner; ownership có thể handoff trực tiếp sang waiter được chọn trước khi task cũ restore priority.
- Mutex API không được gọi từ ISR.
- no-wait -> busy/error;
- blocking -> waiter insert theo effective priority;
- nếu waiter cao hơn owner, recompute/propagate inheritance.
- deadlock detection;
- priority ceiling;
- robust mutex;
- owner-death recovery.


<a id="mo-hinh"></a>
## Mô hình và luồng thực thi

```mermaid
sequenceDiagram
    participant H as High task p=1
    participant M as Mutex
    participant L as Low owner p=5
    participant S as Scheduler
    L->>M: lock()
    H->>M: lock(timeout)
    M->>H: block on waiter list
    M->>L: recompute effective priority = 1
    L->>S: requeued as p=1
    L->>M: unlock()
    M->>H: direct ownership handoff
    M->>L: restore/recompute effective priority
    H->>S: READY at high priority
```

Sơ đồ trên mô tả **semantic boundary**, không thay thế source. Khi debug, nên lần theo node của sơ đồ tới function/source file tương ứng thay vì suy luận từ diagram đơn lẻ.

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
- Audit hiện tại đã chạy `make TARGET=bluepill_f103c8 host-tests`: test suite PASS.
- Host examples `02-kernel-data-structures-host`, `14-memory-allocator-lab`, `16-diagnostics-stress-stabilization` chạy PASS; stress scheduler report 500.000 iteration.
- Không suy ra target runtime PASS từ host test. Cortex-M3 assembly, timing, exception priority, UART/LED và hardware clock vẫn cần cross-build + board validation.


<a id="source-map"></a>
## Source map

- `kernel/src/hr_mutex.c`
- `kernel/internal/hr_mutex_internal.h`
- `kernel/src/hr_kernel.c`
- `tests/host/test_mutex.c`


<a id="references"></a>
## Tài liệu tham khảo

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Nguồn implementation trong repository:**
- `kernel/src/hr_mutex.c`
- `kernel/internal/hr_mutex_internal.h`
- `kernel/src/hr_kernel.c`
- `tests/host/test_mutex.c`
