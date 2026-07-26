# HairRTOS

HairRTOS is an educational fixed-priority RTOS for ARM Cortex-M. The first
target is the STM32F103C8T6 Blue Pill. HairEvent is an optional Event-Driven
framework above the kernel.

## Current status: Phase 12 complete

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
- **Phase 11:** administrative suspend/resume for READY, RUNNING, and BLOCKED tasks;
- **Phase 12:** static one-shot/periodic software timers with task-context callbacks.

Phase 12 keeps SysTick bounded: the interrupt only advances deadlines and wakes
the dedicated timer-service task. Application callbacks run later in task
context, outside kernel critical sections.

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
| 12 | Software timer | ✅ Complete |
| 13 | HairEvent framework | ⬜ Not started |
| 14 | Memory allocator lab | ⬜ Not started |
| 15 | Kernel benchmark | ⬜ Not started |
| 16 | Diagnostics and stabilization | ⬜ Not started |

Detailed deliverables and Definition of Done are in `docs/roadmap.md`.

## Phase 12 components

- opaque statically allocated one-shot and auto-reload timer objects;
- ordered expiration tracking with 32-bit tick-wrap support;
- dedicated timer-service task and binary wake semaphore;
- pending-expiration counting without duplicate intrusive-list insertion;
- start, stop, reset, and period-change task-context APIs;
- callback execution outside SysTick and outside kernel critical sections;
- STM32 target example and host sanitizer validation.

## Examples: host versus target

See `examples/README.md` for the complete matrix.

Run the host demo/tests:

```bash
make phase2-example
make host-tests
```

Build or flash the Phase 12 target example:

```bash
make EXAMPLE=12-software-timer
make EXAMPLE=12-software-timer flash
```

The `EXAMPLE` value must be passed in the same invocation used for `flash`.

## Validation

```bash
make phase0-check
make roadmap-check
make example-layout-check
make host-tests
make phase12-check
```

Read:

- `docs/roadmap.md`
- `docs/task-model.md`
- `docs/software-timer.md`
- `docs/phase12-software-timer.md`
- `examples/README.md`

The next implementation phase is **Phase 13 — HairEvent framework**.
