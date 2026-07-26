# Scheduler Specification

HairRTOS targets fixed-priority preemptive scheduling. **Phase 6 implements the
fixed-priority selection policy in cooperative mode.** Automatic preemption and
tick-based round-robin are introduced later.

## Priority policy

- Smaller number means higher priority.
- Each priority has a FIFO ready queue.
- A ready bitmap records non-empty queues.
- The selected task is the head of the highest-priority non-empty queue.
- The idle task is always READY at the lowest configured priority.
- Registration order matters only among tasks with the same priority.

## Phase 6 cooperative behavior

`hr_task_yield()` pends PendSV. The scheduler rotates only the selected task's
priority queue. If no equal-priority peer exists, the same task remains
selected. A lower-priority task cannot run while any higher-priority task is
READY.

The scheduler chooses a TCB; the Cortex-M port saves and restores registers.

## Later behavior

- Phase 7 adds blocked delay and timeout wake-up.
- Phase 8 requests PendSV when a higher-priority task becomes READY.
- Phase 8 also rotates equal-priority tasks when their time slice expires.

## Invariants

- The RUNNING task is at the head of the highest-priority READY queue.
- Blocked tasks are absent from ready queues.
- Ready bitmap and queue emptiness agree.
- A task cannot exist in multiple ready queues.
- Yield cannot be initiated on behalf of a non-selected lower-priority task.
- The idle task cannot be removed or blocked.
