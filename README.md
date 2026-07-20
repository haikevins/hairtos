# HairRTOS

HairRTOS is an educational fixed-priority preemptive RTOS for ARM Cortex-M.
The first target is the STM32F103C8T6 Blue Pill.

The repository contains two separate layers:

- **HairRTOS Kernel**: tasks, scheduler, timeouts, queues, semaphores, mutexes,
  software timers, ISR-safe APIs, and diagnostics.
- **HairEvent**: optional Event-Driven framework providing events, Active
  Objects, state machines, time events, event pools, and later publish/subscribe.

This archive completes **Phase 0 — Specification and principles**. It defines
architecture, dependency rules, task and scheduler models, interrupt contracts,
memory policy, Event-Driven boundaries, testing rules, and the complete project
layout. It intentionally does not yet contain a runnable kernel.

## Initial target

| Item | Decision |
|---|---|
| Board | Blue Pill |
| MCU | STM32F103C8T6 |
| CPU | ARM Cortex-M3 |
| Language | C11 + GNU Assembly |
| Compiler | arm-none-eabi-gcc |
| Scheduler | Fixed-priority preemptive |
| Equal priority | Round-robin |
| Tick | 1 ms |
| Priorities | 8 |
| Allocation | Static-first |
| FPU / MPU / SMP | Not supported in v1.0 |

Read `docs/architecture.md`, `docs/task-model.md`, `docs/scheduler.md`,
`docs/interrupt-model.md`, and `docs/event-framework.md` first.

## Phase 0 checks

```bash
make phase0-check
make tree
```
