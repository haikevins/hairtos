# Blocking contract

> **Phạm vi:** Implementation `hairtos 1.0.0-rc1`, bao gồm source, config, build graph và host-test evidence hiện có.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [Next →](mutex.md)

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

Blocking contract là cơ chế chung mà task delay, queue, semaphore và mutex dùng để đưa current task ra khỏi ready set, gắn wait/timeout metadata và đưa task trở lại READY qua đúng một wake path.


<a id="implementation"></a>
## Implementation trong repository

Implementation hiện tại gồm:

- wait kind;
- object wait ordering;
- timeout cleanup;
- suspend behavior;
- ISR behavior;
- direct handoff/ownership;
- effective priority impact;
- host tests cho event-vs-timeout race.


<a id="mo-hinh"></a>
## Mô hình và luồng thực thi

**Blocking entry**

```mermaid
flowchart TB
    CALL["Blocking API"] --> FAST{"Fast path?"}
    FAST -->|"Success"| RET["Return"]
    FAST -->|"Must wait"| DETACH["Remove current from ready set"]
    DETACH --> WAIT["Attach object wait metadata"]
    WAIT --> TIMEOUT["Optional timeout node"]
```

**Single-winner wake**

```mermaid
flowchart TB
    EVENT["Object wake or timeout"] --> CLEAN["Detach remaining wait metadata"]
    CLEAN --> RESULT["Publish one wait_result"]
    RESULT --> READY["Make task READY"]
    READY --> PEND["PendSV if required"]
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

- `README.md`
- `CMakeLists.txt`
- `config/hairtos_config.h`


<a id="references"></a>
## Tài liệu tham khảo


**Nguồn implementation trong repository:**
- `README.md`
- `CMakeLists.txt`
- `config/hairtos_config.h`
