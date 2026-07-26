# HairRTOS

HairRTOS is an educational fixed-priority RTOS for ARM Cortex-M. The first
target is the STM32F103C8T6 Blue Pill. HairEvent is an optional Event-Driven
framework above the kernel.

## Current status: Phase 9 complete

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
  among equal-priority tasks;
- **Phase 9:** statically allocated FIFO queues, blocking send/receive, finite
  timeout, wait forever, direct handoff, and ISR-safe non-blocking variants.

Phase 9 adds task-to-task and ISR-to-task message transfer. Queue wait lists are
priority ordered and FIFO among equal priorities. A sender or receiver blocks
without busy-waiting and is returned to READY by the matching operation or by a
timeout.

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
| 10 | Semaphore and mutex | ⬜ Not started |
| 11 | Suspend/resume | ⬜ Not started |
| 12 | Software timer | ⬜ Not started |
| 13 | HairEvent framework | ⬜ Not started |
| 14 | Memory allocator lab | ⬜ Not started |
| 15 | Kernel benchmark | ⬜ Not started |
| 16 | Diagnostics and stabilization | ⬜ Not started |

Detailed deliverables and Definition of Done are in `docs/roadmap.md`.

## Phase 9 components

- opaque statically allocated `hr_queue_t` control block;
- caller-provided ring-buffer storage with fixed item size and capacity;
- FIFO enqueue/dequeue with wrap-around indices;
- non-blocking `HR_NO_WAIT`, finite timeout, and `HR_WAIT_FOREVER`;
- priority-ordered blocked sender and receiver wait lists;
- direct sender-to-receiver handoff when a receiver is already blocked;
- atomic refill of a freed queue slot from the highest-priority blocked sender;
- timeout removal from both the object wait list and global timeout list;
- `hr_queue_send_from_isr()` and `hr_queue_receive_from_isr()` with a
  higher-priority-task-woken result;
- Phase 9 STM32 producer/consumer example and host sanitizer coverage.

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
make EXAMPLE=09-queue-blocking-ipc flash
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
make phase9-check
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
- `docs/phase9-queue-blocking-ipc.md`
- `examples/README.md`

The next implementation phase is **Phase 10 — Semaphore and mutex**.
