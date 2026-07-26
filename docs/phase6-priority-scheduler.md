# Phase 6 — Fixed-Priority Scheduler

## Goal

Integrate the Phase 2 ready data structures into an explicit scheduler policy.
The scheduler chooses the highest-priority READY task and keeps FIFO ordering
among tasks with equal priority.

## Priority convention

```text
priority 0 = highest urgency
priority 7 = lowest configured urgency
```

The idle task occupies the lowest configured priority and is selected only when
no more urgent task is READY.

## Scheduler representation

```c
typedef struct
{
    hr_ready_set_t ready;
} hr_scheduler_t;
```

The ready set contains one intrusive FIFO list per priority and a bitmap of
non-empty queues.

## Policy operations

```c
void hr_scheduler_init(hr_scheduler_t *scheduler);
hr_status_t hr_scheduler_add_ready(hr_scheduler_t *scheduler,
                                   hr_ready_node_t *node);
hr_ready_node_t *hr_scheduler_select_highest(
    const hr_scheduler_t *scheduler);
hr_status_t hr_scheduler_yield_current(hr_scheduler_t *scheduler,
                                       hr_ready_node_t *current);
```

`hr_scheduler_yield_current()` is valid only for the currently selected
highest-priority node. It rotates that priority queue when an equal-priority peer
exists. It never selects a lower-priority task while a higher-priority task is
READY.

## Current limitations

Phase 6 is cooperative:

- task switching happens only after `hr_task_yield()`;
- all application tasks are registered before kernel start;
- there is no blocking delay;
- there is no runtime READY transition from an ISR;
- there is no automatic preemption;
- there is no tick-based time slicing.

Preemption and round-robin time slicing are Phase 8 features.

## Invariants

- the RUNNING task is the scheduler's highest selected node;
- smaller priority number wins;
- equal-priority tasks preserve insertion order;
- yield rotates only the highest-priority FIFO;
- a lower-priority task cannot force scheduler rotation;
- the ready bitmap matches queue occupancy;
- a task appears in at most one ready queue.

## Validation

Host tests verify:

- registration order does not override priority;
- equal-priority FIFO rotation;
- lower-priority starvation while high tasks remain READY;
- single highest task yields to itself;
- non-selected task yield is rejected;
- ready count, bitmap, and list invariants.

The target example verifies the policy through SVC and PendSV on Cortex-M3.
