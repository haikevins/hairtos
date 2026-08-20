# Known limitations — v1

> These are actual limitations of `1.0.0-rc1`, not a hypothetical bug list.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](glossary.md) · [Next →](source-map.md)

## Kernel/architecture

- Single-core only; no SMP.
- No FPU context save/restore; the Cortex-M3 target has no FPU.
- No MPU isolation/user mode separation.
- Critical sections use a global PRIMASK mask; there is no BASEPRI interrupt-ceiling contract yet.
- No tickless idle; 1 kHz SysTick baseline.
- No general dynamic kernel allocator/task create/delete runtime.
- No task-deletion lifecycle.
- No automatic deadlock detection/prevention.

## `haievent`

- Flat FSM only; no hierarchical parent chain/LCA transition.
- No deferred/recall/history state.
- One task per Active Object; no shared executor.
- Pub/sub subscriber matrix fixed-size compile/caller storage.
- The event pool uses fixed-size blocks; events larger than the block size cannot be allocated.

## Platform

- `bluepill_f103c8` is currently the only complete target.
- The driver layer is intentionally small and is not a general-purpose vendor-independent HAL.
- Blocking UART logging can affect timing.
- DWT/marker benchmark support is target-specific evidence.

## Validation

- Host tests provide strong coverage of generic C logic but do not replace hardware exception/peripheral tests.
- Host validation does not replace target cross-build and OpenOCD/hardware validation.
- The project does not claim safety certification or hard real-time certification.

## Related Roadmap Items

HSMs, a trace ring, a second target, an interrupt ceiling, and tickless idle are proposals under `docs/09-version2/`.
