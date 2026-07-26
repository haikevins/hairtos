# Task Suspend and Resume Policy

Phase 11 separates administrative suspension from synchronization blocking.

## Public API

```c
hr_status_t hr_task_suspend(hr_task_t *task);
hr_status_t hr_task_resume(hr_task_t *task);
```

Both functions are task-context APIs. ISR calls return `HR_ERROR_FROM_ISR`.
The idle task cannot be suspended. Suspension is not nestable.

## READY and RUNNING tasks

A READY task is removed from its ready queue and enters `SUSPENDED`. A running
task may suspend itself; PendSV then selects the next ready task. Resuming either
case inserts the task at the back of its effective-priority ready queue. A
resumed task preempts the current task only when it has strictly higher priority.

## BLOCKED tasks

HairRTOS preserves the original wait-list and timeout membership. The task state
becomes `SUSPENDED`, while the TCB records that its underlying state is BLOCKED.

- Resume before completion restores `BLOCKED`; the original wait continues.
- Event or timeout completion removes the original wait and records its result,
  but the task remains `SUSPENDED`.
- A later resume changes it to `READY`.

This policy preserves synchronization results, but an event handed directly to
a suspended waiter is reserved for that task until it is resumed. Applications
should therefore avoid indefinitely suspending a task that owns a mutex or is
first in an object wait list.

## State model

```text
READY   --suspend--> SUSPENDED(READY) --resume--> READY
RUNNING --suspend--> SUSPENDED(READY) --resume--> READY
BLOCKED --suspend--> SUSPENDED(BLOCKED)
SUSPENDED(BLOCKED) --resume before event--> BLOCKED
SUSPENDED(BLOCKED) --event/timeout--> SUSPENDED(READY)
SUSPENDED(READY)   --resume--> READY
```
