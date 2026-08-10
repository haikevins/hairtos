# Kernel roadmap Version 2

## K1 — Failure policy

Thay các infinite-spin paths khó debug:

- task entry return;
- AO internal dispatch failure;
- severe internal invariant failure.

Bằng policy:

```text
record diagnostics
invoke hook
enter controlled panic/halt
```

Task termination/delete vẫn có thể chưa cần.

## K2 — Interrupt ceiling

Cortex-M port:

- thêm `HR_CFG_MAX_SYSCALL_INTERRUPT_PRIORITY` hoặc abstraction tương đương;
- BASEPRI critical section;
- validate ISR priority nếu có thể;
- docs phân biệt kernel-aware/high-urgency ISR;
- benchmark critical latency.

Giữ PRIMASK fallback cho architecture/target không có priority mask nếu cần.

## K3 — Tickless idle

Thêm generic next-deadline query/service.

Port low-power contract:

```text
prepare sleep
program wake source
sleep
measure elapsed
resume/advance time
```

Phải tương tác đúng với:

- timeout list;
- software timer;
- time slicing;
- pending IRQ;
- wrap.

## K4 — Extended uptime

Không nhất thiết đổi `hr_tick_t` thành 64-bit vì làm tăng cost/ABI. Có thể giữ scheduling tick 32-bit và thêm diagnostic monotonic uptime 64-bit.

## K5 — Synchronization diagnostics

Mutex:

- optional ownership graph snapshot;
- detect obvious wait cycle trong diagnostics/debug build;
- report owner/waiters/effective priority.

Không cần automatic deadlock recovery.

## K6 — Lightweight signaling

Event flags/task notification chỉ nên vào 2.x nếu use-case chứng minh queue/semaphore overhead không phù hợp.

Nếu thêm, thiết kế blocking contract phải reuse wait/timeout machinery, không tạo state path riêng khó validate.

## K7 — Scheduler

Không đổi sang EDF trong 2.0. Fixed-priority là identity của baseline.

Có thể tối ưu highest-ready scan bằng CLZ/bit operation theo port sau khi correctness baseline giữ nguyên, nhưng default C fallback vẫn cần.

## K8 — FPU/MPU

FPU context là port capability, không kernel policy. Có thể đưa Cortex-M4F port vào 2.0 nếu second target dùng nó.

MPU isolation lớn hơn nhiều; nên để 2.1+ trừ khi target thứ hai yêu cầu.
