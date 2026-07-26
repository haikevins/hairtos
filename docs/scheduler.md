# Scheduler Specification

HairRTOS uses preemptive fixed-priority scheduling with one FIFO ready queue per
priority. Phase 8 adds strict higher-priority preemption and tick-driven
round-robin among equal-priority tasks.

## Priority policy

- Smaller number means higher priority.
- Each priority has a FIFO ready queue.
- A ready bitmap records non-empty queues.
- The selected task is the head of the highest-priority non-empty queue.
- `HR_CFG_IDLE_PRIORITY` is reserved for the kernel idle task.
- Registration order matters only among tasks with the same priority.

## Preemption

After timeout wake-up, SysTick compares the highest READY priority with the
currently RUNNING task. A strictly higher-priority task causes PendSV to be
pended. A lower-priority or equal-priority wake-up does not use the preemption
path.

The interrupted task remains at the head of its own priority queue and retains
its unused quantum, so it resumes before later peers at that priority.

## Round-robin

When at least two tasks share the highest priority, SysTick decrements the
RUNNING task's `time_slice_remaining`. At zero, the queue head moves to the tail
and PendSV restores the next peer. The quantum is configured by
`HR_CFG_TIME_SLICE_TICKS`.

The quantum reloads after expiration, explicit yield, blocking, and timeout
wake-up. With no equal-priority peer, the current task keeps running and its
quantum is reloaded.

## Yield

`hr_task_yield()` pends PendSV. If the current task remains the highest READY
task, its equal-priority queue is rotated. If a higher-priority task is already
READY, PendSV selects it without rotating the lower-priority current queue.

## Delay and timeout

A delaying task is removed from its ready queue before it becomes BLOCKED. When
its timeout expires, SysTick returns it to the correct priority queue and
requests preemption when required.

## Invariants

- Exactly one task is RUNNING.
- Every READY or RUNNING task is linked in exactly one ready queue.
- BLOCKED tasks are absent from ready queues and linked in the timeout structure
  when their wait is finite.
- Ready bitmap and queue emptiness agree.
- A task cannot exist in multiple ready queues.
- The idle task cannot block and owns the reserved idle priority.
