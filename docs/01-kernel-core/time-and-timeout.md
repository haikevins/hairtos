# Time and Timeouts

> **Scope:** `hairtos 1.0.0-rc1` implementation, including the current source, configuration, build graph, and host-test evidence.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](task-model.md)

## Table of Contents

- [Overview and Core Concepts](#overview)
- [Implementation in the Repository](#implementation)
- [Execution Model and Runtime Flow](#runtime-model)
- [Ownership, Concurrency, and Invariants](#invariants)
- [Failure Modes and Limitations](#failure)
- [Validation and Verification](#validation)
- [Source map](#source-map)
- [References](#references)

<a id="overview"></a>
## Overview and Core Concepts

Timeouts use two sorted intrusive lists: `current` for deadlines before the next wrap and `overflow` for deadlines after the `uint32_t` tick wraps. When `now` wraps past `last_tick`, the lists are swapped. This keeps deadline comparisons simple while correctly supporting 32-bit tick wrap-around.


<a id="implementation"></a>
## Implementation in the Repository

The current implementation includes:

- Each blocked task has exactly one timeout node, and that node belongs to exactly one of the two timeout lists while a finite timeout is active.
- `HR_WAIT_FOREVER` requires no timeout node; `HR_NO_WAIT` does not block.
- Object wakeup and timeout wakeup race on the same wait state; the winning path must remove the task consistently from both the wait list and timeout list.
- `delay_until()` uses an absolute periodic reference to reduce phase drift compared with adding a delay after each actual execution.
- Wrap-around behavior is unit-tested directly in `test_timeout.c`.


<a id="runtime-model"></a>
## Execution Model and Runtime Flow

**Timeout insertion**

```mermaid
flowchart TB
    BLOCK["Finite-timeout block"] --> WAKE["Compute wake_tick"]
    WAKE --> CHOOSE{"Wrapped deadline?"}
    CHOOSE -->|"No"| CURRENT["Insert current list"]
    CHOOSE -->|"Yes"| OVERFLOW["Insert overflow list"]
```

**Tick expiry path**

```mermaid
flowchart TB
    TICK["Kernel tick"] --> WRAP{"Tick wrapped?"}
    WRAP -->|"Yes"| SWAP["Swap timeout lists"]
    WRAP -->|"No"| EXPIRE["Expire due nodes"]
    SWAP --> EXPIRE
    EXPIRE --> READY["Cleanup wait + READY"]
```

The corresponding functions and source files are listed in the Source Map section.

<a id="invariants"></a>
## Ownership, Concurrency, and Invariants

The following baseline invariants apply to this topic:

- A public opaque object is valid only after a successful create/init operation and when its magic/internal state matches the contract.
- An intrusive node may be linked into exactly one list at a time; remove/timeout/wake paths must leave the node in a consistent unlinked state.
- A thread API may block only while the kernel is RUNNING and the caller is not in ISR context; ISR APIs must be non-blocking and use `higher_priority_task_woken` when a switch should be deferred to PendSV.
- Critical sections currently use PRIMASK on Cortex-M3, globally masking interrupts for a short bounded interval; therefore critical-section code must remain bounded and must not invoke operations that can block.
- Ready/wait policy uses **effective priority** while mutex priority inheritance is active; base priority remains the task's configured priority.
- Static-first does not mean “no lifetime”: caller-owned TCB, stack, queue storage, and event-pool storage must outlive every object that still references them.

<a id="failure"></a>
## Failure Modes and Limitations

- `hairtos 1.0.0-rc1` is single-core and provides no SMP, FPU context management, MPU isolation, or general-purpose dynamic kernel heap.
- The current interrupt-masking model uses PRIMASK; the repository does not yet define a BASEPRI ceiling contract for applications with complex ISR-priority schemes.
- Tickless idle is not implemented; the current time model uses a 1 kHz tick on the reference target.
- `haievent` v1 provides a flat state machine and one task per AO; HSMs, deferred events, and a shared executor are Version 2 roadmap items.
- A successful build/link does not by itself prove real-time timing or race-free behavior on hardware; target tests and measurements remain necessary.

<a id="validation"></a>
## Validation and Verification

- The repository's host suite is built with GCC, AddressSanitizer, UndefinedBehaviorSanitizer, and `ctest`.
- Host validation baseline: `make TARGET=bluepill_f103c8 host-tests` PASS.
- Host examples `02-kernel-data-structures-host`, `14-memory-allocator-lab`, and `16-diagnostics-stress-stabilization` pass; the scheduler stress test reports 500,000 iterations.
- Do not infer target-runtime PASS from host tests. Cortex-M3 assembly, timing, exception priorities, UART/LED behavior, and hardware clocks still require cross-build and board validation.


<a id="source-map"></a>
## Source map

- `kernel/src/hr_timeout.c`
- `kernel/src/hr_kernel.c`
- `kernel/src/hr_time.c`
- `tests/host/test_timeout.c`


<a id="references"></a>
## References

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Implementation sources in the repository:**
- `kernel/src/hr_timeout.c`
- `kernel/src/hr_kernel.c`
- `kernel/src/hr_time.c`
- `tests/host/test_timeout.c`
