# HairRTOS

HairRTOS is an educational fixed-priority preemptive RTOS for ARM Cortex-M.
The first target is the STM32F103C8T6 Blue Pill. HairEvent is an optional
Event-Driven framework above the kernel.

## Current status: Phase 2 complete

Implemented phases:

- **Phase 0:** specification and principles;
- **Phase 1:** register-level Blue Pill bare-metal foundation;
- **Phase 2:** intrusive lists and host-tested kernel data structures.

Phase 2 adds no task execution or context switching yet. The target firmware is
still the Phase 1 blink/UART example.

## Phase 2 components

- circular intrusive doubly linked list;
- FIFO ready queue per priority;
- ready priority bitmap;
- highest-priority ready-node selection;
- same-priority queue rotation prototype;
- priority-ordered wait list with equal-priority FIFO;
- dual ordered timeout lists with 32-bit tick-wrap handling;
- host test harness with ASan and UBSan.

## Commands

Build the STM32 Phase 1 target:

```bash
make
make flash
```

Run Phase 2 host tests:

```bash
make host-tests
```

Validate all completed phases:

```bash
make phase0-check
make phase1-check
make roadmap-check
make phase2-check
```

## Architecture

```text
Application
    -> HairEvent framework
    -> HairRTOS kernel
    -> Cortex-M port
    -> STM32F1 SoC
    -> Blue Pill board and drivers
```

Read:

- `docs/architecture.md`
- `docs/roadmap.md`
- `docs/phase1-baremetal-foundation.md`
- `docs/phase2-kernel-data-structures.md`
- `docs/testing-guide.md`

The next implementation phase is **Phase 3 — TCB and initial task stack**.
