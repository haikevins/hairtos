# Task Model

A task consists of one TCB and one statically supplied stack.

## States

Core execution states:

```text
CREATED -> READY -> RUNNING
RUNNING -> READY       yield or preemption
RUNNING -> BLOCKED     delay or object wait
BLOCKED -> READY       event or timeout
```

Phase 11 adds the administrative state:

```text
READY/RUNNING -> SUSPENDED
SUSPENDED     -> READY
```

Phase 11 permits suspension of a BLOCKED task without moving its wait or
timeout nodes. The TCB records whether resume should restore BLOCKED or READY.
See `task-suspend-resume.md`.

## TCB requirements

The saved stack pointer is the first field. The Phase 11 TCB includes:

- stack bounds;
- name and state;
- base and effective priorities;
- wake tick and time slice;
- ready/wait/all-task list nodes;
- waiting object;
- critical nesting;
- runtime counter;
- stack high-water mark;
- optional ownership data for held mutexes.

## Stack contract

- Task code uses PSP; exceptions use MSP.
- Stack is aligned to 8 bytes.
- Initial xPSR sets the Thumb bit.
- R0 contains the task argument.
- LR points to a task-return error handler.
- Stack memory is filled with `0xA5` and protected by a guard word.

## Priority contract

`base_priority` is configured by the application. `effective_priority` is used
by the scheduler and may be raised temporarily by priority inheritance.

## Blocking contract

A task may block only in task context, with the kernel running, outside a
critical section, and through an API documented as blocking.

A BLOCKED task waits for an event or timeout. A SUSPENDED task waits only for an
explicit resume operation.

Only internal transition helpers may change task state. Drivers and
applications never write TCB state directly.
