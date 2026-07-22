# Task Suspend and Resume Plan

## Purpose

Suspend/resume is an administrative task-control mechanism and is not the same
as blocking on a queue, semaphore, mutex, delay, or timeout.

## Planned API

```c
hr_status_t hr_task_suspend(hr_task_t *task);
hr_status_t hr_task_resume(hr_task_t *task);
hr_status_t hr_task_resume_from_isr(hr_task_t *task,
                                    bool *higher_priority_task_woken);
```

The exact ISR API remains optional until the interrupt contract is implemented
and tested.

## Planned state

```text
CREATED
READY
RUNNING
BLOCKED
SUSPENDED
```

## Core semantics

- Suspending a READY task removes it from the ready queue.
- Self-suspending the RUNNING task requests a context switch.
- A SUSPENDED task does not wake because a previous timeout expires.
- Resume changes a valid SUSPENDED task to READY.
- Resume requests preemption if the resumed task has higher effective priority.
- Suspending the idle task is forbidden.
- Suspending an invalid or already suspended task returns an explicit status or
  triggers a debug assertion according to the final API contract.

## Blocked-task policy

The first implementation should use the simpler policy:

```text
A BLOCKED task cannot be suspended directly.
```

This avoids a task belonging simultaneously to a wait object, timeout structure,
and suspended set. Supporting suspended-blocked combinations may be considered
only after the basic state machine is proven.

## Required tests

- suspend READY task;
- self-suspend RUNNING task;
- resume task;
- resume higher-priority task and preempt;
- suspend idle task rejected;
- repeated suspend/resume rejected;
- blocked-task suspension policy enforced;
- resume from ISR, if implemented.
