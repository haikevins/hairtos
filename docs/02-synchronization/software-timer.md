# Software timer

> **Scope:** `hairtos 1.0.0-rc1` implementation, including the current source, configuration, build graph, and host-test evidence.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](semaphore.md) · [Next →](suspend-resume.md)

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

Software timers do not execute callbacks in SysTick. The tick ISR only transfers expired timers to a pending queue and signals the timer-service task; callbacks execute in task context. Each timer tracks `pending_count` so multiple expirations are not silently collapsed while the service task is delayed.


<a id="implementation"></a>
## Implementation in the Repository

The current implementation includes:

- A timer is static opaque storage containing a name, period, `auto_reload`, callback, argument, and timeout node.
- The timer-service task is created lazily when the timer subsystem is first initialized and uses priority/stack settings from configuration.
- The expiry ISR path only updates state/pending information and wakes the service task; callbacks never execute in Handler mode.
- A one-shot timer becomes inactive after expiry; a periodic timer is re-armed for its configured period.
- `stop/reset/change_period` deliberately handle both an active timeout node and any pending callbacks.
- change period.
- an active timer has correct timeout-list membership;
- a pending node is never double-linked;
- callbacks do not execute in ISR context;
- the period is valid.


<a id="runtime-model"></a>
## Execution Model and Runtime Flow

**Expiry handoff**

```mermaid
sequenceDiagram
    participant ST as SysTick ISR
    participant TL as Timer list
    participant TS as Timer service
    ST->>TL: advance time
    TL-->>ST: expired timers
    ST->>TS: queue pending work
    ST->>TS: wake service task
```

**Callback execution**

```mermaid
sequenceDiagram
    participant TS as Timer service
    participant CB as User callback
    TS->>TS: pop pending timer
    TS->>CB: invoke callback
    CB-->>TS: return
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

- `kernel/src/hr_timer.c`
- `kernel/internal/hr_timer_internal.h`
- `tests/host/test_timer.c`


<a id="references"></a>
## References

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Implementation sources in the repository:**
- `kernel/src/hr_timer.c`
- `kernel/internal/hr_timer_internal.h`
- `tests/host/test_timer.c`
