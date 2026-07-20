# Scheduler Specification

HairRTOS uses fixed-priority preemptive scheduling.

- Smaller number means higher priority.
- Each priority has a FIFO ready queue.
- A ready bitmap records non-empty queues.
- The next task is the head of the highest-priority non-empty queue.
- Equal-priority tasks rotate when their time slice expires.
- The idle task is always ready at the lowest reserved priority.

The scheduler selects a TCB; it does not save or restore CPU registers.

A higher-priority task becoming READY requests PendSV. The switch occurs only
after the active exception completes.

`hr_task_yield()` rotates only the current priority queue. It never allows a
lower-priority task to run while a higher-priority task remains READY.

## Invariants

- RUNNING task is at the head of its ready queue.
- Blocked tasks are absent from ready queues.
- Ready bitmap and queue emptiness agree.
- A task cannot exist in multiple ready queues.
- Idle task cannot be removed or blocked.
