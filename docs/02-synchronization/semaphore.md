# Semaphore

> **Scope:** `hairtos 1.0.0-rc1` implementation, including the current source, configuration, build graph, and host-test evidence.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](queue.md) · [Next →](software-timer.md)

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

A semaphore is a synchronization counter with no ownership. A counting semaphore stores `count/max_count`; a binary semaphore is simply configured with max=1. `give` prefers waking an existing waiter rather than incrementing the count when a task is waiting.


<a id="implementation"></a>
## Implementation in the Repository

The current implementation includes:

- `take` consumes a token when count > 0; if no token is available, it may block according to the timeout.
- `give` with a waiting task transfers forward progress directly to that waiter; only when there is no waiter is the count incremented.
- `give_from_isr` is ISR-safe and reports any required context switch through its output flag.
- Semaphores track no owner and provide no priority inheritance; use a mutex when ownership-aware mutual exclusion is required.
- Giving when the count is already at maximum and no waiter exists returns `HR_ERROR_SEMAPHORE_FULL`.
- no-wait -> `HR_ERROR_SEMAPHORE_EMPTY`;
- finite/forever -> block on the waiter list.
- `count <= max_count`;
- max > 0;
- waiter ordering remains correct;
- a token is not both incremented and handed off in the same give operation.


<a id="runtime-model"></a>
## Execution Model and Runtime Flow

```mermaid
flowchart TB
    TAKE["Take"] --> TOK{"count > 0?"}
    TOK -->|"Yes"| DEC["Decrement count"]
    TOK -->|"No + wait"| WAIT["Block task"]
    GIVE["Give"] --> W{"Waiter exists?"}
    W -->|"Yes"| WAKE["Wake highest-priority waiter"]
    W -->|"No"| INC["Increment count"]
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

- `kernel/src/hr_semaphore.c`
- `kernel/internal/hr_semaphore_internal.h`
- `tests/host/test_semaphore.c`


<a id="references"></a>
## References

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Implementation sources in the repository:**
- `kernel/src/hr_semaphore.c`
- `kernel/internal/hr_semaphore_internal.h`
- `tests/host/test_semaphore.c`
