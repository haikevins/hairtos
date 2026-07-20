# Task Model

A task consists of one TCB and one statically supplied stack.

## States

```text
CREATED -> READY -> RUNNING
RUNNING -> READY       yield or preemption
RUNNING -> BLOCKED     delay or object wait
BLOCKED -> READY       event or timeout
```

## TCB requirements

The saved stack pointer is the first field. Planned fields include stack bounds,
name, state, base and effective priorities, wake tick, time slice, ready/wait/all
list nodes, waiting object, critical nesting, runtime counter, and stack
high-water mark.

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

Only internal transition helpers may change task state. Drivers and
applications never write TCB state directly.
