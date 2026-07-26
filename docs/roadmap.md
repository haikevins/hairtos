# HairRTOS Official Roadmap

## Status rule

This document defines future work but does not make a phase complete. A phase is
complete only when its code, tests, focused example, documentation, and target
validation satisfy its Definition of Done.

Current completed phase:

```text
Phase 13 — HairEvent framework
```

## Phase 0 — Specification and principles

### Goals

- define architecture and dependency direction;
- fix the first hardware target;
- define task, scheduler, interrupt, timeout, memory, and event contracts;
- establish coding and testing rules.

### Completion criteria

- architecture documents exist;
- configuration decisions are explicit;
- public/internal boundaries are defined;
- later work can be implemented without guessing core policies.

### Status

Complete.

---

## Phase 1 — Bare-metal foundation

### Goals

- boot STM32F103 without HAL or SPL;
- initialize `.data` and `.bss`;
- configure the Blue Pill memory map;
- establish the 72 MHz clock;
- provide GPIO, UART, and a temporary bare-metal SysTick time base;
- support build, flash, and debug.

### Deliverables

- startup and vector table;
- linker script;
- board and clock support;
- register-level GPIO, UART, and SysTick drivers;
- blink/heartbeat example;
- GNU Arm and Clang validation paths.

### Completion criteria

- compile and link without warnings;
- produce ELF, BIN, HEX, and MAP files;
- vector table is located at the Flash base;
- LED and UART can be validated on target hardware.

### Status

Complete in source and build validation. Physical board validation remains a
user-side activity.

---

## Phase 2 — Intrusive lists and kernel data structures

**Status: Complete.**

### Goals

Implement platform-independent structures before writing context-switch code.

### Deliverables

- intrusive doubly linked list;
- FIFO ready-list primitive;
- priority ready-set prototype;
- wait list ordered by effective priority;
- timeout ordering structure;
- host-side unit-test framework;
- ASan and UBSan test build.

### Required tests

- empty, one-node, head, tail, and middle operations;
- double insertion and double removal detection;
- FIFO ordering;
- waiter priority ordering;
- equal-priority FIFO behavior;
- timeout ordering around tick wrap.

### Definition of Done

No MCU-specific header is included. All host tests pass with warnings treated as
errors and sanitizers enabled.

---

## Phase 3 — TCB and initial task stack

**Status: Complete.**

### Goals

Define static tasks and construct the initial Cortex-M exception frame.

### Deliverables

- TCB layout;
- static task creation API;
- stack bounds and guard initialization;
- Cortex-M3 initial stack-frame builder;
- task-return panic handler;
- GDB inspection guide.

### Required validation

- stack is 8-byte aligned;
- R0 contains the argument;
- PC points to the task entry;
- xPSR Thumb bit is set;
- LR points to the task-return handler.

---

## Phase 4 — Start first task using SVC

**Status: Complete.**

### Goals

Leave `main()` on MSP and start the first selected task on PSP.

### Deliverables

- SVC handler;
- first-task startup path;
- idle task;
- kernel lifecycle transition to RUNNING.

### Definition of Done

The first task runs on PSP and `hr_kernel_start()` never returns during normal
operation.

---

## Phase 5 — PendSV cooperative context switch

**Status: Complete.**

### Goals

Switch between statically created tasks only when they yield.

### Deliverables

- PendSV save/restore of R4-R11;
- current/next TCB handoff;
- `hr_task_yield()`;
- cooperative switching example.

### Definition of Done

Multiple tasks retain local variables and execution positions through a long
context-switch stress run.

---

## Phase 6 — Fixed-priority scheduler

**Status: Complete.**

### Goals

Select the highest-priority ready task and preserve FIFO order among equals.

### Deliverables

- ready queues per priority;
- ready bitmap;
- effective-priority selection;
- centralized task-state transition helpers.

### Definition of Done

Scheduler policy is host-tested independently from PendSV assembly.

---

## Phase 7 — SysTick, delay, and timeout base

**Status: Complete.**

### Goals

Replace the temporary Phase 1 timing role with the kernel time service.

### Deliverables

- kernel tick;
- blocked timeout structure;
- `hr_task_delay()`;
- `hr_task_delay_until()`;
- finite and infinite waits;
- tick-wrap tests.

### Definition of Done

Delay is blocking rather than busy-waiting, and periodic delay avoids cumulative
drift.

---

## Phase 8 — Preemption and round-robin

**Status: Complete.**

### Goals

Make a newly ready higher-priority task run immediately after exception return,
and rotate equal-priority tasks by time slice.

### Deliverables

- preemption decision;
- deferred PendSV request;
- equal-priority time slicing;
- idle-task scheduling rules.

### Definition of Done

Higher-priority tasks do not wait for lower-priority tasks to yield. Equal
priorities do not starve.

---

## Phase 9 — Queue and blocking IPC

**Status: Complete.**

### Goals

Implement fixed-size static queues for task-to-task and ISR-to-task transfer.

### Deliverables

- ring-buffer storage;
- blocking/non-blocking send and receive;
- finite timeout and wait forever;
- `_from_isr` variants;
- highest-priority waiter wake-up.

### Required tests

- FIFO payload order;
- full and empty queues;
- sender and receiver blocking;
- timeout races;
- ISR wake-up;
- equal-priority waiter FIFO behavior.

### Definition of Done

Queue payload order remains FIFO across ring-buffer wrap. Full senders and empty
receivers leave the ready set, then return because of direct handoff, available
capacity/data, or timeout. ISR operations never block and report whether a
higher-priority waiter became READY.

---

## Phase 10 — Semaphore, mutex, and priority inheritance

**Status: Complete.**

### Goals

Add synchronization primitives without hiding ownership rules.

### Deliverables

- binary semaphore;
- counting semaphore;
- non-recursive mutex;
- owner validation;
- basic priority inheritance;
- effective-priority restoration across multiple held mutexes.

### Definition of Done

A priority-inversion scenario is reproduced and then corrected by inheritance.

---

## Phase 11 — Task suspend/resume

**Status: Complete.**

### Goals

Add an explicit administrative state separate from waiting on an object.

### Deliverables

- `SUSPENDED` task state;
- `hr_task_suspend()`;
- `hr_task_resume()`;
- task-context-only resume;
- behavior for self-suspend;
- invalid-state protection.

### Important distinction

A blocked task waits for an event or timeout. A suspended task does not become
ready because a timeout or synchronization event occurs; only resume changes it
back to READY.

### Definition of Done

Suspend/resume works for READY, RUNNING, and BLOCKED tasks according to
`docs/task-suspend-resume.md`, including deferred readiness after wait completion.

---

## Phase 12 — Software timer service

**Status: Complete.**

### Goals

Provide one-shot and periodic timers without executing application callbacks in
SysTick context.

### Deliverables

- timer object;
- ordered expiration structure;
- pending-expiration list and timer-service wake semaphore;
- serialized task-context timer control operations;
- timer service task;
- start, stop, reset, and period-change operations.

### Definition of Done

Callbacks never execute in SysTick context or while kernel structures are locked.
Callback jitter measurement remains part of physical target validation.

---

## Phase 13 — HairEvent framework

**Status: Complete.**

### Goals

Build Event-Driven Programming above the completed kernel primitives without
moving state-handler execution into ISR or timer-service context.

### Deliverables

1. reserved and application signals;
2. immutable static events;
3. fixed-block dynamic event pools and reference counting;
4. direct task-context and ISR-safe event post;
5. statically allocated Active Objects;
6. flat state machines with ENTRY, EXIT, INIT, and run-to-completion transitions;
7. software-timer-backed time events;
8. fixed-capacity publish/subscribe;
9. six focused STM32 examples and host sanitizer tests.

Hierarchical state machines remain an explicit post-Phase-13 extension. Flat
transition and ownership semantics are stabilized first.

### Definition of Done

`13-01-event-post` uses EXTI0 software interrupt to post an event. The Active
Object wakes after exception return, handles the event run-to-completion in its
own task, releases ownership when required, and blocks again. Time-event and
publish/subscribe examples validate the remaining framework layers.

---

## Phase 14 — Memory allocator lab

### Goals

Study dynamic-memory mechanisms without making the production kernel depend on
them.

### Deliverables

- first-fit allocator;
- aligned block headers;
- block splitting;
- adjacent-block coalescing;
- free-list validation;
- invalid-free and double-free detection;
- fragmentation and usage statistics;
- host tests and an optional target demonstration.

### Boundary

The v1.0 kernel stays static-first. The allocator lab is optional and isolated
under `labs/memory-allocator/`.

---

## Phase 15 — Kernel benchmarks

### Goals

Measure behavior only after correctness and stress stability are established.

### Metrics

- SVC first-task startup latency;
- PendSV context-switch latency;
- scheduler decision time;
- SysTick-to-task wake-up latency;
- queue send-to-receive latency;
- event post-to-dispatch latency;
- software-timer jitter;
- maximum critical-section duration;
- stack high-water mark;
- code size and static RAM use.

### Measurement methods

- DWT cycle counter on Cortex-M3;
- GPIO pulse and logic analyzer;
- buffered UART output after measurement;
- repeated samples with min, max, mean, and selected percentiles.

### Definition of Done

Measurement overhead and conditions are documented, and benchmark code is not
part of the normal kernel hot path.

---

## Phase 16 — Diagnostics, stress, and stabilization

### Goals

Prepare an educational v1.0 release.

### Deliverables

- stack-overflow hook;
- kernel panic path;
- runtime counters;
- queue and synchronization diagnostics;
- long-duration stress tests;
- API reference;
- porting guide;
- Cortex-M0 proof-of-port;
- release checklist.

### v1.0 exit criteria

- no known kernel invariant failure in stress runs;
- all host and target tests pass;
- misuse assertions are documented;
- every public API states task/ISR context and blocking behavior;
- Cortex-M3 target is stable;
- kernel remains independent of STM32F1 and HairEvent.
