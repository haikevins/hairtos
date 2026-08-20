# Time Event

> **Scope:** `hairtos 1.0.0-rc1` implementation, including the current source, configuration, build graph, and host-test evidence.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](state-machine.md)

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

A Time Event is an adapter between a hairtos software timer and an Active Object event queue. The timer callback creates/posts a static timeout event to the AO; because the callback runs in the timer-service task, this is a task-context post rather than ISR dispatch.


<a id="implementation"></a>
## Implementation in the Repository

The current implementation includes:

- A time event embeds a kernel timer and stores its target AO/signal.
- Periodic/one-shot semantics are delegated to `hr_timer_*`.
- If the AO queue cannot accept a timeout event, `dropped_count` increments with saturation at UINT32_MAX.
- Disarm/arm/rearm/change-period operations map directly to the corresponding timer APIs.
- A timeout event does not perform a state transition by itself; the AO state handler determines the signal's meaning.
- kernel timer;
- target AO;
- embedded static event;
- dropped counter;
- validity/armed metadata.


<a id="runtime-model"></a>
## Execution Model and Runtime Flow

```mermaid
flowchart TB
    TICK["Kernel tick"] --> KT["Timer expiry"]
    KT --> TS["Timer-service task"]
    TS --> TE["Time-event callback"]
    TE --> POST["Post timeout event"]
    POST --> AO["AO RTC dispatch"]
    POST -->|"failure"| DROP["dropped_count++"]
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

- `haievent/src/he_time_event.c`
- `kernel/src/hr_timer.c`
- `haievent/src/he_active.c`


<a id="references"></a>
## References


**Implementation sources in the repository:**
- `haievent/src/he_time_event.c`
- `kernel/src/hr_timer.c`
- `haievent/src/he_active.c`
