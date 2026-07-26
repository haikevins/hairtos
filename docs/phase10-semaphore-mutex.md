# Phase 10 — Semaphore, Mutex, and Priority Inheritance

## Scope

Phase 10 adds synchronization primitives above the Phase 9 blocking core:

- binary and counting semaphores;
- task-context semaphore take/give;
- ISR-safe semaphore give;
- non-recursive and recursive mutex creation;
- mutex ownership validation;
- direct ownership handoff to the highest-priority waiter;
- priority inheritance and restoration across multiple held mutexes;
- finite timeout and wait-forever support.

## Semaphore model

A counting semaphore stores `count`, `max_count`, and one priority-ordered wait
list. A binary semaphore is the special case `max_count == 1`.

When a task takes an unavailable semaphore, it leaves the ready set and enters
`BLOCKED`. A give operation directly wakes the highest-priority waiter instead
of incrementing the count first. Equal-priority waiters remain FIFO.

`hr_semaphore_give_from_isr()` never blocks. It reports whether the unblocked
task has a strictly higher effective priority than the interrupted task. The
ISR can then request PendSV through `hr_port_yield_from_isr()`.

## Mutex model

A mutex tracks:

- owner task;
- recursion count;
- recursive/non-recursive policy;
- priority-ordered waiters;
- an intrusive node in the owner's held-mutex list.

Unlock is legal only for the current owner. If waiters exist, ownership moves
directly to the highest-priority waiter before that waiter becomes READY.

## Priority inheritance

Each task has a base priority and an effective priority. The effective priority
is the highest urgency required by:

1. the task's base priority; and
2. the first waiter of every mutex currently held by the task.

Because lower numeric values represent higher priority, the effective priority
is the minimum of those values. Recalculation scans all held mutexes, so
unlocking one mutex does not incorrectly restore the base priority while
another held mutex still has a higher-priority waiter.

If an inherited owner is itself blocked on another mutex, the change propagates
through the ownership chain, bounded by `HR_CFG_MAX_TASKS` to protect against a
corrupt or cyclic chain.

## Timeout behavior

A mutex waiter with a finite timeout is linked to both the mutex wait list and
the global timeout list. On timeout, the generic kernel wait-cleanup callback
removes its contribution from the owner's inherited priority before the waiter
returns to READY with `HR_ERROR_TIMEOUT`.

## Examples

```bash
make EXAMPLE=10-01-semaphore-from-isr flash
make EXAMPLE=10-02-mutex-priority-inheritance flash
```

The semaphore example uses an EXTI0 software interrupt, so no external button
is required. The mutex example reproduces priority inversion with High=1,
Medium=3, and Low=5.

## Boundaries

Phase 10 does not yet provide task suspend/resume, software timers, deadlock
detection, or priority-ceiling protocols. Mutex APIs are task-only; semaphore
give is the only synchronization operation provided for ISR context.
