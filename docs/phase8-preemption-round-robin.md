# Phase 8 — Preemption and Round-Robin

## Scope

Phase 8 changes the Phase 7 scheduler from timeout-assisted cooperative
operation into a preemptive fixed-priority scheduler with tick-driven time
slicing among equal priorities.

Implemented:

- strict higher-priority preemption;
- deferred context switch through PendSV;
- configurable equal-priority time slice;
- per-task remaining quantum;
- idle preemption when an application task becomes ready;
- `HR_CFG_IDLE_PRIORITY` reserved for the kernel idle task;
- preservation of a preempted task's remaining quantum;
- quantum reload after yield, blocking, timeout wake-up, or expiration.

## Preemption decision

After timeout processing, SysTick compares the highest ready priority with the
currently running task. A switch is requested only when the ready task has a
strictly smaller priority number.

```text
running priority 5
    + priority 1 task becomes READY
    -> pend PendSV
    -> priority 1 task runs after exception return
```

A lower-priority wake-up does not disturb a higher-priority running task.

## Round-robin

When at least two READY/RUNNING tasks share the highest priority, SysTick
decrements the current task's quantum. At zero, the current queue head moves to
the tail and PendSV restores the next peer.

```text
priority 3 queue: A -> B
one quantum expires
priority 3 queue: B -> A
```

No explicit `hr_task_yield()` is required for the CPU-bound tasks in the Phase
8 example.

## Deferred switching

SysTick and task APIs never save task registers directly. They record a switch
reason and pend PendSV. PendSV remains the only handler that saves/restores
PSP and R4-R11.

PendSV masks interrupts around the C scheduler selector so ready queues,
state transitions, and the current TCB pointer are updated atomically.

## Phase-specific examples

The Phase 7 target is compiled with `HR_CFG_PREEMPTION=0` and
`HR_CFG_TIME_SLICING=0`, preserving its original cooperative timeout behavior.
The Phase 8 target and host tests use the enabled defaults.

## Phase boundary

Phase 8 does not yet add queue objects or IPC blocking. Those are Phase 9.
