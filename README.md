# HairRTOS

HairRTOS is an educational fixed-priority preemptive RTOS for ARM Cortex-M.
The first target is the STM32F103C8T6 Blue Pill. HairEvent is an optional
Event-Driven framework above the kernel.

## Current status: Phase 3 complete

Implemented phases:

- **Phase 0:** specification and principles;
- **Phase 1:** register-level Blue Pill bare-metal foundation;
- **Phase 2:** intrusive lists and host-tested kernel data structures;
- **Phase 3:** opaque static TCB storage and Cortex-M3 initial task stack.

Phase 3 creates task objects but does not run them. Starting the first task
through SVC belongs to Phase 4.

## Roadmap

Status legend:

- ✅ **Complete** — implementation, documentation, and the required validation
  for the phase are present in this archive.
- ⬜ **Not started** — only specifications or placeholder files may exist; the
  phase must not be treated as implemented.

| Phase | Scope | Status |
|---:|---|---|
| 0 | Specification | ✅ Complete |
| 1 | Bare-metal foundation | ✅ Complete |
| 2 | Intrusive list and kernel data structures | ✅ Complete |
| 3 | TCB and initial task stack | ✅ Complete |
| 4 | Start first task using SVC | ⬜ Not started |
| 5 | PendSV cooperative context switch | ⬜ Not started |
| 6 | Priority scheduler | ⬜ Not started |
| 7 | SysTick and delay | ⬜ Not started |
| 8 | Preemption and round-robin | ⬜ Not started |
| 9 | Queue and blocking | ⬜ Not started |
| 10 | Semaphore and mutex | ⬜ Not started |
| 11 | Suspend/resume | ⬜ Not started |
| 12 | Software timer | ⬜ Not started |
| 13 | HairEvent framework | ⬜ Not started |
| 14 | Memory allocator lab | ⬜ Not started |
| 15 | Kernel benchmark | ⬜ Not started |
| 16 | Diagnostics and stabilization | ⬜ Not started |

The detailed Definition of Done for each phase is documented in
`docs/roadmap.md`.

## Phase 3 components

- opaque, aligned, statically allocated `hr_task_t` storage;
- internal Task Control Block with saved SP as its first field;
- task metadata, state, priorities, intrusive nodes, and diagnostics fields;
- stack fill pattern and guard word;
- 8-byte-aligned initial Cortex-M3 frame;
- R0 argument, PC task entry, LR task-return trap, and xPSR Thumb state;
- task validation, stack guard, and high-watermark APIs;
- host tests with ASan/UBSan;
- target example `03-static-task-stack`.

## Examples: host versus target

The examples are not all built the same way:

- **Host examples** run natively on Ubuntu and cannot be flashed to STM32.
- **Target examples** are cross-compiled for STM32F103 and may be flashed only
  when their phase is implemented.
- Placeholder examples document future phases and are not runnable yet.

See `examples/README.md` for the complete host/target table and exact commands.

## Commands

Run host tests and the Phase 2 host demonstration:

```bash
make host-tests
make phase2-example
```

Build target examples:

```bash
make EXAMPLE=01-baremetal-foundation
make EXAMPLE=03-static-task-stack
```

Flash a target example by passing `EXAMPLE` in the same command:

```bash
make EXAMPLE=01-baremetal-foundation flash
make EXAMPLE=03-static-task-stack flash
```

Do not use `make flash` with `02-kernel-data-structures-host`; it is a native
Ubuntu program, not STM32 firmware.

Validate completed phases:

```bash
make phase0-check
make phase1-check
make roadmap-check
make example-layout-check
make phase2-check
make phase3-check
```

Read:

- `docs/phase1-baremetal-foundation.md`
- `docs/phase2-kernel-data-structures.md`
- `docs/phase3-tcb-initial-stack.md`
- `docs/roadmap.md`
- `docs/example-index.md`
- `examples/README.md`

The next implementation phase is **Phase 4 — Start first task using SVC**.
