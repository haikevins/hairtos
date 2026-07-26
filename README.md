# HairRTOS

HairRTOS is an educational fixed-priority RTOS for ARM Cortex-M. The first
target is the STM32F103C8T6 Blue Pill. HairEvent is an optional Event-Driven
framework above the kernel.

## Current status: Phase 7 complete

Implemented phases:

- **Phase 0:** specification and principles;
- **Phase 1:** register-level Blue Pill bare-metal foundation;
- **Phase 2:** intrusive lists and host-tested kernel data structures;
- **Phase 3:** opaque static TCB and Cortex-M3 initial task stack;
- **Phase 4:** first-task startup through SVC from MSP to PSP;
- **Phase 5:** cooperative task-to-task context switching through PendSV;
- **Phase 6:** fixed-priority scheduler with FIFO queues for equal priorities;
- **Phase 7:** kernel SysTick, blocking task delay, periodic delay, and timeout
  wake-up.

Phase 7 removes a delayed task from the ready queues and runs another task or
the idle task. When a timeout expires while idle is running, SysTick pends
PendSV so the woken application task can resume. General preemption of an
arbitrary running lower-priority task and tick-driven equal-priority time
slicing belong to Phase 8.

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
| 8 | Preemption and round-robin | ⬜ Not started |
| 9 | Queue and blocking | ⬜ Not started |
| 10 | Semaphore and mutex | ⬜ Not started |
| 11 | Suspend/resume | ⬜ Not started |
| 12 | Software timer | ⬜ Not started |
| 13 | HairEvent framework | ⬜ Not started |
| 14 | Memory allocator lab | ⬜ Not started |
| 15 | Kernel benchmark | ⬜ Not started |
| 16 | Diagnostics and stabilization | ⬜ Not started |

Detailed deliverables and Definition of Done are in `docs/roadmap.md`.

## Phase 7 components

- strong kernel `SysTick_Handler` for RTOS examples;
- separate bare-metal SysTick handler retained for Phase 1–6 examples;
- monotonic 1 kHz kernel tick;
- two-epoch timeout lists for 32-bit tick wrap;
- `hr_task_delay()` for relative blocking delays;
- `hr_task_delay_until()` for drift-resistant periodic releases;
- `RUNNING -> BLOCKED -> READY` transitions;
- basic PRIMASK critical-section port primitives;
- timeout-driven switch from idle through PendSV.

## Examples: host versus target

See `examples/README.md` for the complete environment matrix.

Run the host example and completed host tests:

```bash
make phase2-example
make host-tests
```

Build or flash implemented target examples:

```bash
make EXAMPLE=01-baremetal-foundation flash
make EXAMPLE=03-static-task-stack flash
make EXAMPLE=04-start-first-task flash
make EXAMPLE=05-cooperative-context-switch flash
make EXAMPLE=06-priority-scheduler flash
make EXAMPLE=07-task-delay-timeout flash
```

The `EXAMPLE` value must be passed in the same command used for `flash`.

## Validation

```bash
make phase0-check
make phase1-check
make roadmap-check
make example-layout-check
make phase2-check
make phase3-check
make phase4-check
make phase5-check
make phase6-check
make phase7-check
```

Read:

- `docs/phase1-baremetal-foundation.md`
- `docs/phase2-kernel-data-structures.md`
- `docs/phase3-tcb-initial-stack.md`
- `docs/phase4-start-first-task-svc.md`
- `docs/phase5-cooperative-context-switch.md`
- `docs/phase6-priority-scheduler.md`
- `docs/phase7-systick-delay.md`
- `examples/README.md`

The next implementation phase is **Phase 8 — Preemption and round-robin**.
