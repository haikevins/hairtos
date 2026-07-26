# Phase 5 — Cooperative Context Switch

## Scope

Phase 5 adds explicit task-to-task context switching. It does not add timer-driven
preemption, blocking delays, queues, or ISR-safe synchronization.

Two or more READY tasks at the same highest priority can cooperate by calling
`hr_task_yield()`.

## Yield path

```text
Task A on PSP
    -> hr_task_yield()
    -> set SCB->ICSR.PENDSVSET
    -> return from hr_task_yield()
    -> PendSV exception entry
    -> hardware saves R0-R3, R12, LR, PC, xPSR on Task A PSP
    -> PendSV saves R4-R11
    -> store Task A PSP in TCB A
    -> rotate the highest ready queue
    -> select Task B and update current TCB
    -> restore R4-R11 from Task B PSP
    -> exception return restores the hardware frame
    -> Task B continues
```

## Cooperative policy

Phase 5 deliberately demonstrates two equal-priority tasks. Explicit yield
rotates only the highest ready queue. A lower-priority task does not run while a
higher-priority task remains READY.

Full fixed-priority scheduler behavior and its dedicated example are completed
in Phase 6. Timer-driven time slicing remains Phase 8.

## Context frame

Cortex-M hardware saves:

```text
R0-R3, R12, LR, PC, xPSR
```

PendSV saves and restores:

```text
R4-R11
```

The internal TCB keeps `stack_pointer` at offset zero so assembly can save and
load PSP without depending on the rest of the TCB layout.

## Interrupt behavior

PendSV is configured at the lowest priority. `hr_port_request_context_switch()`
only sets the pending bit; it does not switch stacks inside the C function.

Phase 5 does not yet expose general critical sections or FromISR kernel APIs.

## Definition of Done

- strong PendSV handler is linked;
- R4-R11 save and restore are visible in target disassembly;
- PSP is stored in and loaded from the TCB;
- two tasks alternate only after explicit yield;
- both tasks preserve independent local variables;
- Phase 0-4 regression tests still pass.
