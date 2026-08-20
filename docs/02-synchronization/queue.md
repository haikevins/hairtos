# Queue

> **Scope:** `hairtos 1.0.0-rc1` implementation, including the current source, configuration, build graph, and host-test evidence.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](mutex.md) · [Next →](semaphore.md)

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

The queue is a bounded FIFO backed by caller-owned byte storage plus an opaque control block. Beyond a conventional circular buffer, the implementation maintains priority-ordered sender/receiver wait lists and supports direct handoff so an item can move directly between waiting producer and consumer when possible.


<a id="implementation"></a>
## Implementation in the Repository

The current implementation includes:

- Queue storage is not dynamically allocated; `item_size × capacity` bytes are caller-owned and must remain valid for the queue's entire lifetime.
- Task APIs support timeouts; ISR APIs are always non-blocking and report `higher_priority_task_woken` rather than scheduling directly.
- Waiters are ordered by effective priority and FIFO within the same priority through wait-list insertion order.
- When a receiver is already waiting, send may copy directly into the receive buffer instead of enqueueing and then dequeueing; similarly, receive may take data directly from a blocked sender.
- Full/empty queues with `HR_NO_WAIT` return immediately; blocking is valid only while the kernel is RUNNING and the caller is not in ISR context.
- `count <= capacity`;
- head/tail indices remain within range;
- sender waits when it cannot send;
- receiver waits when it cannot receive;
- wait-buffer lifetime remains valid while the task is blocked;
- `item_size`/`capacity` do not change after creation.


<a id="runtime-model"></a>
## Execution Model and Runtime Flow

**Send path**

```mermaid
flowchart TB
    S["Sender"] --> Q{"Receiver waiting?"}
    Q -->|"Yes"| H["Direct handoff"]
    Q -->|"No"| CAP{"FIFO space?"}
    CAP -->|"Yes"| ENQ["Enqueue item"]
    CAP -->|"No + wait"| SW["Block sender"]
```

**Receive path**

```mermaid
flowchart TB
    R["Receiver"] --> E{"FIFO item?"}
    E -->|"Yes"| DEQ["Dequeue item"]
    E -->|"No"| SS{"Sender waiting?"}
    SS -->|"Yes"| DH["Direct handoff"]
    SS -->|"No + wait"| RW["Block receiver"]
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

- `kernel/src/hr_queue.c`
- `kernel/internal/hr_queue_internal.h`
- `kernel/src/hr_wait.c`
- `tests/host/test_queue.c`


<a id="references"></a>
## References

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Implementation sources in the repository:**
- `kernel/src/hr_queue.c`
- `kernel/internal/hr_queue_internal.h`
- `kernel/src/hr_wait.c`
- `tests/host/test_queue.c`
