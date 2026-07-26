# Scheduler Specification

HairRTOS uses fixed-priority scheduling with one FIFO ready queue per priority.
Phase 7 adds BLOCKED tasks and timeout wake-up while preserving the cooperative
Phase 6 boundary for normal running tasks.

## Priority policy

- Smaller number means higher priority.
- Each priority has a FIFO ready queue.
- A ready bitmap records non-empty queues.
- The selected task is the head of the highest-priority non-empty queue.
- The idle task is always READY at the lowest configured priority.
- Registration order matters only among tasks with the same priority.

## Yield

`hr_task_yield()` pends PendSV. If the current task remains the highest READY
task, its equal-priority queue is rotated. If a higher-priority task is already
READY, PendSV selects that task without rotating the lower-priority current
queue.

## Delay and timeout

A delaying task is removed from its ready queue before it becomes BLOCKED. When
its timeout expires, SysTick returns it to the correct priority queue.

Phase 7 pends PendSV immediately when an application task wakes while idle is
running. If a non-idle task is already running, the woken task remains READY
until a cooperative scheduling point. Phase 8 generalizes this into automatic
higher-priority preemption and adds tick-based equal-priority rotation.

## Invariants

- Exactly one task is RUNNING.
- Every READY or RUNNING task is linked in exactly one ready queue.
- BLOCKED tasks are absent from ready queues and linked in the timeout structure
  when their wait is finite.
- Ready bitmap and queue emptiness agree.
- A task cannot exist in multiple ready queues.
- The idle task cannot block.
