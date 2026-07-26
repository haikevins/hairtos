# HairRTOS

HairRTOS is an educational fixed-priority RTOS for ARM Cortex-M. The first
target is the STM32F103C8T6 Blue Pill. HairEvent is the optional static-first
Event-Driven framework implemented above the kernel.

## Current status: Phase 15 complete

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
- **Phase 14:** isolated fixed-block pool and first-fit heap allocator laboratory;
- **Phase 15:** DWT/GPIO kernel benchmark with deferred UART reporting.

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
| 15 | Kernel benchmark | ✅ Complete |
| 16 | Diagnostics and stabilization | ⬜ Not started |

Detailed deliverables and Definition of Done are in `docs/roadmap.md`.

## Phase 15 benchmark

The dedicated target image measures:

- SVC startup;
- DWT read overhead and critical-section cost;
- fixed-priority scheduler short/long scan positions;
- queue, semaphore, mutex, and software-timer command paths;
- two-switch PendSV yield round trip;
- queue wake/preempt and HairEvent post/dispatch round trips;
- periodic software-timer interval and absolute jitter;
- linked Flash/static RAM and task stack high-water marks.

Samples are stored in static RAM. UART output starts only after collection, and
PB0 provides an external active-high timing marker. No benchmark code is linked
into normal examples.

## Examples: host versus target

See `examples/README.md` for the complete matrix.

Host demos and tests:

```bash
make phase2-example
make phase14-lab
make host-tests
```

Build or flash the Phase 15 benchmark:

```bash
make EXAMPLE=15-kernel-benchmark
make EXAMPLE=15-kernel-benchmark flash
```

The `EXAMPLE` value must be passed in the same invocation used for `flash`.

## Validation

```bash
make phase0-check
make roadmap-check
make example-layout-check
make host-tests
make phase15-check
```

Read:

- `docs/roadmap.md`
- `docs/kernel-benchmark.md`
- `docs/phase15-kernel-benchmark.md`
- `benchmarks/kernel/README.md`
- `examples/README.md`

The next implementation phase is **Phase 16 — Diagnostics and stabilization**.
