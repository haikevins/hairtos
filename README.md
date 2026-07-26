# HairRTOS

HairRTOS is an educational fixed-priority RTOS for ARM Cortex-M. The first
target is the STM32F103C8T6 Blue Pill. HairEvent is an optional Event-Driven
framework above the kernel.

## Current status: Phase 11 complete

Implemented phases:

- **Phase 0:** specification and principles;
- **Phase 1:** register-level Blue Pill bare-metal foundation;
- **Phase 2:** intrusive lists and host-tested kernel data structures;
- **Phase 3:** opaque static TCB and Cortex-M3 initial task stack;
- **Phase 4:** first-task startup through SVC from MSP to PSP;
- **Phase 5:** cooperative task-to-task context switching through PendSV;
- **Phase 6:** fixed-priority scheduler with FIFO queues for equal priorities;
- **Phase 7:** kernel SysTick, blocking delay, periodic delay, and timeout wake-up;
- **Phase 8:** strict higher-priority preemption and tick-driven round-robin;
- **Phase 9:** static FIFO queues with blocking, timeout, direct handoff, and ISR API;
- **Phase 10:** semaphores, mutex ownership, and priority inheritance;
- **Phase 11:** administrative suspend/resume for READY, RUNNING, and BLOCKED tasks.

Phase 11 keeps suspension separate from object waiting. A suspended task never
enters the ready queue because a timeout or synchronization event completed; an
explicit `hr_task_resume()` is required.

## Roadmap

| Phase | Scope | Status |
|---:|---|---|
| 0 | Specification | ✅ Complete |
| 1 | Bare-metal foundation | ✅ Complete |
| 2 | Intrusive list and kernel data structures | ✅ Complete |
| 3 | TCB and initial task stack | ✅ Complete |
| 4 | Start first task using SVC | ✅ Complete |
| 5 | PendSV cooperative context switch | ✅ Complete |
| 6 | Priority scheduler | ✅ Complete |
| 7 | SysTick and delay | ✅ Complete |
| 8 | Preemption and round-robin | ✅ Complete |
| 9 | Queue and blocking | ✅ Complete |
| 10 | Semaphore and mutex | ✅ Complete |
| 11 | Suspend/resume | ✅ Complete |
| 12 | Software timer | ⬜ Not started |
| 13 | HairEvent framework | ⬜ Not started |
| 14 | Memory allocator lab | ⬜ Not started |
| 15 | Kernel benchmark | ⬜ Not started |
| 16 | Diagnostics and stabilization | ⬜ Not started |

Detailed deliverables and Definition of Done are in `docs/roadmap.md`.

## Phase 11 components

- `hr_task_suspend()` and `hr_task_resume()` task-context APIs;
- self-suspend of the currently running task through PendSV;
- removal and reinsertion of READY tasks from the priority ready queues;
- preservation of BLOCKED task wait-list and timeout state;
- deferred readiness when a suspended wait completes;
- immediate preemption after resuming a strictly higher-priority task;
- idle-task, duplicate-state, and ISR-context protection;
- STM32 target example and host sanitizer validation.

## Examples: host versus target

See `examples/README.md` for the complete matrix.

Run the host demo/tests:

```bash
make phase2-example
make host-tests
```

Build or flash the Phase 11 target example:

```bash
make EXAMPLE=11-task-suspend-resume
make EXAMPLE=11-task-suspend-resume flash
```

The `EXAMPLE` value must be passed in the same invocation used for `flash`.

## Validation

```bash
make phase0-check
make roadmap-check
make example-layout-check
make host-tests
make phase11-check
```

Read:

- `docs/roadmap.md`
- `docs/task-model.md`
- `docs/task-suspend-resume.md`
- `docs/phase11-suspend-resume.md`
- `examples/README.md`

The next implementation phase is **Phase 12 — Software timer**.
