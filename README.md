# HairRTOS

HairRTOS is an educational fixed-priority RTOS for ARM Cortex-M. The first
target is the STM32F103C8T6 Blue Pill. HairEvent is an optional Event-Driven
framework above the kernel.

## Current status: Phase 8 complete

Implemented phases:

- **Phase 0:** specification and principles;
- **Phase 1:** register-level Blue Pill bare-metal foundation;
- **Phase 2:** intrusive lists and host-tested kernel data structures;
- **Phase 3:** opaque static TCB and Cortex-M3 initial task stack;
- **Phase 4:** first-task startup through SVC from MSP to PSP;
- **Phase 5:** cooperative task-to-task context switching through PendSV;
- **Phase 6:** fixed-priority scheduler with FIFO queues for equal priorities;
- **Phase 7:** kernel SysTick, blocking task delay, periodic delay, and timeout
  wake-up;
- **Phase 8:** strict higher-priority preemption and tick-driven round-robin
  among equal-priority tasks.

Phase 8 makes a newly READY higher-priority task preempt the running task after
exception return. CPU-bound equal-priority tasks rotate automatically after
`HR_CFG_TIME_SLICE_TICKS` and no longer need to call `hr_task_yield()`.

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
| 9 | Queue and blocking | ⬜ Not started |
| 10 | Semaphore and mutex | ⬜ Not started |
| 11 | Suspend/resume | ⬜ Not started |
| 12 | Software timer | ⬜ Not started |
| 13 | HairEvent framework | ⬜ Not started |
| 14 | Memory allocator lab | ⬜ Not started |
| 15 | Kernel benchmark | ⬜ Not started |
| 16 | Diagnostics and stabilization | ⬜ Not started |

Detailed deliverables and Definition of Done are in `docs/roadmap.md`.

## Phase 8 components

- strict fixed-priority preemption when a higher-priority task becomes READY;
- deferred scheduling through PendSV;
- equal-priority round-robin controlled by `HR_CFG_TIME_SLICE_TICKS`;
- per-task remaining quantum;
- idle preemption when an application task wakes;
- the lowest configured priority reserved for the idle task;
- atomic PendSV selector update with interrupts masked;
- Phase 7 delay and timeout behavior retained; its example disables Phase 8 scheduling features at build time.

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
make EXAMPLE=08-preemption-round-robin flash
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
make phase8-check
```

Read:

- `docs/phase1-baremetal-foundation.md`
- `docs/phase2-kernel-data-structures.md`
- `docs/phase3-tcb-initial-stack.md`
- `docs/phase4-start-first-task-svc.md`
- `docs/phase5-cooperative-context-switch.md`
- `docs/phase6-priority-scheduler.md`
- `docs/phase7-systick-delay.md`
- `docs/phase8-preemption-round-robin.md`
- `examples/README.md`

The next implementation phase is **Phase 9 — Queue and blocking IPC**.
