# HairRTOS Architecture

## Purpose

HairRTOS is an educational RTOS that is small enough to study down to the
Cortex-M exception frame, but structured for testing and later porting.

## Product split

### HairRTOS Kernel

Owns task stacks and TCBs, fixed-priority scheduling, preemption, time slicing,
timeouts, queues, semaphores, mutexes, software timers, ISR-safe services, and
diagnostics.

### HairEvent

An optional layer above the kernel. It owns events, signals, Active Objects,
state machines, time events, event pools, and later publish/subscribe.

## Layering

```text
Application
  -> HairEvent
  -> HairRTOS public API
  -> Kernel internals
  -> Cortex-M port
  -> STM32F1 SoC
  -> Blue Pill board/drivers
  -> Hardware
```

Lower layers must never include or call higher layers.

## Phase 0 decisions

- Single core only.
- ARM Cortex-M3 first.
- Tasks run privileged and use PSP.
- Exceptions use MSP.
- No FPU context and no MPU isolation in v1.0.
- Static allocation first.
- Eight fixed priority levels.
- Priority number 0 is the highest.
- Preemption and equal-priority round-robin are enabled.
- Kernel tick is 1 ms.
- Event framework is optional and separate from the kernel.

## Architectural invariants

1. Exactly one task is RUNNING after the kernel starts.
2. A task appears in at most one ready queue.
3. A blocked task has exactly one wait reason.
4. Scheduler policy is in `kernel`; register context is in `arch`.
5. ISR APIs never block.
6. Application callbacks never run while kernel structures are locked.
7. Application signals are never interpreted by the kernel.
8. Drivers never modify a TCB or scheduler list.
9. Event handlers run to completion and return quickly.
10. Public API contracts state context, blocking, timeout, and ownership rules.

## Non-goals for v1.0

SMP, POSIX compatibility, dynamic task deletion, a general-purpose heap, MPU
user mode, networking, file systems, loadable modules, and tickless idle.
