# HairRTOS

HairRTOS is an educational fixed-priority RTOS for ARM Cortex-M. The first
target is the STM32F103C8T6 Blue Pill. HairEvent is the optional static-first
Event-Driven framework implemented above the kernel.

## Current status: Phase 14 complete

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
- **Phase 12:** static one-shot/periodic software timers with task-context callbacks;
- **Phase 13:** HairEvent event pools, Active Objects, flat state machines, time events,
  and publish/subscribe;
- **Phase 14:** isolated fixed-block pool and first-fit heap allocator laboratory.

HairEvent keeps state handlers run-to-completion. ISRs and software-timer
callbacks only post events. Each Active Object owns a HairRTOS task, event queue,
state machine, stack, and private context; its handler executes later in that
Active Object task context.

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
| 13 | HairEvent framework | ✅ Complete |
| 14 | Memory allocator lab | ✅ Complete |
| 15 | Kernel benchmark | ⬜ Not started |
| 16 | Diagnostics and stabilization | ⬜ Not started |

Detailed deliverables and Definition of Done are in `docs/roadmap.md`.

## Phase 14 components

- isolated `labs/memory-allocator/` implementation with no kernel dependency;
- fixed-block pool with aligned blocks and explicit capacity;
- first-fit variable-size heap with aligned headers and payloads;
- block splitting and bidirectional adjacent-block coalescing;
- invalid-pointer, middle-pointer, double-free, exhaustion, and corruption checks;
- internal/external fragmentation and usage statistics;
- deterministic randomized host testing under ASan/UBSan;
- standalone native demo and STM32 UART demonstration.

The allocator remains an educational lab. HairRTOS and HairEvent continue to use
caller-provided static storage.

## Examples: host versus target

See `examples/README.md` for the complete matrix.

Run the host demos and tests:

```bash
make phase2-example
make phase14-lab
make host-tests
```

Build or flash the Phase 14 target example:

```bash
make EXAMPLE=14-memory-allocator-lab
make EXAMPLE=14-memory-allocator-lab flash
```

The `EXAMPLE` value must be passed in the same invocation used for `flash`.

## Validation

```bash
make phase0-check
make roadmap-check
make example-layout-check
make host-tests
make phase14-check
```

Read:

- `docs/roadmap.md`
- `docs/memory-allocator-lab.md`
- `docs/phase14-memory-allocator-lab.md`
- `labs/memory-allocator/README.md`
- `examples/README.md`

The next implementation phase is **Phase 15 — Kernel benchmark**.
