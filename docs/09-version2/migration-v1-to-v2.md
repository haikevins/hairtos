# Migration v1 → v2

> **Status: FUTURE DESIGN.** Nội dung này không phải capability của `hairtos 1.0.0-rc1`.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](kernel-roadmap.md) · [Next →](portability-roadmap.md)

## Mục lục

- [Baseline v1](#baseline)
- [Mục tiêu](#goals)
- [Design constraints](#constraints)
- [Evidence để được coi là hoàn thành](#evidence)
- [Migration/risk](#migration)
- [References](#references)

<a id="baseline"></a>
## Baseline v1

Version 2 phải bắt đầu từ behavior v1 đang có: static object ownership, fixed-priority scheduler, intrusive ready/wait/timeout structures, direct-handoff IPC, one-task-per-AO, flat FSM, target manifest và host sanitizer tests. “Thiết kế mới” không được xóa evidence tốt chỉ để đổi kiến trúc.

<a id="goals"></a>
## Mục tiêu

- Migration phải incremental: build v1 workload trên v2 core, sau đó opt-in HSM/tickless/trace.
- Static object ownership và existing task/IPC semantics nên giữ để giảm churn.
- Behavior thay đổi phải có before/after table và test tương ứng.
- interrupt priority config;
- task-return behavior;
- config macro mới;
- target manifest v2.

<a id="constraints"></a>
## Design constraints

- Không merge API/header trước implementation + tests.
- Mọi feature phải ghi memory cost, runtime cost, ISR implication và failure modes.
- Generic kernel không được nhận dependency vào STM32/board registers.
- Static-first vẫn là default; dynamic behavior nếu thêm phải explicit, bounded và opt-in.
- Version 2 docs phải giữ nhãn proposal cho tới khi capability matrix/source/test được cập nhật.

<a id="evidence"></a>
## Evidence để được coi là hoàn thành

Một mục roadmap chỉ chuyển sang implemented khi có đủ:

1. source implementation trong module đúng layer;
2. unit/host tests hoặc compile probes tương ứng;
3. target evidence nếu feature phụ thuộc architecture/hardware;
4. compatibility/migration note;
5. benchmark/overhead evidence nếu tác động timing hoặc RAM/Flash;
6. cập nhật capability matrix và API docs.

<a id="migration"></a>
## Migration / risk

Rủi ro lớn nhất là scope creep làm mất tính audit được của một RTOS nhỏ. Migration nên opt-in theo feature và giữ v1 workload chạy được càng lâu càng tốt. HSM/tickless/trace/target 2 phải được tách phase để khi regression xuất hiện có thể khoanh vùng nguyên nhân.

<a id="references"></a>
## References

- [`../00-overview/capability-matrix.md`](../00-overview/capability-matrix.md) — baseline capability.
- [`../01-kernel-core/kernel-invariants.md`](../01-kernel-core/kernel-invariants.md) — invariant v1 không được phá ngầm.
- [`../06-testing-and-quality/validation-baseline.md`](../06-testing-and-quality/validation-baseline.md) — evidence baseline.
- [Semantic Versioning 2.0.0](https://semver.org/)
