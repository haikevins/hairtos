# Known limitations — v1

> Đây là limitation thực tế của `1.0.0-rc1`, không phải bug list giả định.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](glossary.md) · [Next →](source-map.md)

## Kernel/architecture

- Single-core only; no SMP.
- No FPU context save/restore; target Cortex-M3 không có FPU.
- No MPU isolation/user mode separation.
- Critical section dùng PRIMASK global mask; chưa có BASEPRI interrupt ceiling contract.
- No tickless idle; 1 kHz SysTick baseline.
- No general dynamic kernel allocator/task create/delete runtime.
- Task delete lifecycle không có.
- Automatic deadlock detection/prevention không có.

## `haievent`

- Flat FSM only; no hierarchical parent chain/LCA transition.
- No deferred/recall/history state.
- One task per Active Object; no shared executor.
- Pub/sub subscriber matrix fixed-size compile/caller storage.
- Event pool fixed-block; event lớn hơn block không allocate được.

## Platform

- Chỉ `bluepill_f103c8` là target hoàn chỉnh hiện tại.
- Driver layer rất nhỏ, không phải vendor-independent HAL tổng quát.
- UART log blocking có thể ảnh hưởng timing.
- Benchmark DWT/marker là target-specific evidence.

## Validation

- Host tests mạnh ở generic C nhưng không thay hardware exception/peripheral tests.
- Audit hiện tại không cross-build target vì thiếu ARM toolchain/OpenOCD.
- Project không tuyên bố safety certification/hard real-time certification.

## Roadmap liên quan

HSM, trace ring, second target, interrupt ceiling và tickless idle nằm trong `docs/09-version2/` với status proposal.
