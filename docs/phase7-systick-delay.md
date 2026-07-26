# Phase 7 — SysTick and Delay

## Scope

Phase 7 replaces the temporary bare-metal millisecond role with a kernel-owned
1 kHz tick for RTOS examples.

Implemented:

- strong kernel `SysTick_Handler`;
- monotonic 32-bit kernel tick;
- two-epoch timeout list for tick wrap;
- `hr_task_delay()`;
- `hr_task_delay_until()`;
- RUNNING -> BLOCKED -> READY transitions;
- timeout wake-up from idle through deferred PendSV;
- basic nested PRIMASK critical sections at the port boundary.

## Delay flow

```text
RUNNING task
    -> remove from ready queue
    -> insert timeout node
    -> state BLOCKED
    -> pend PendSV
    -> another ready task or idle runs
```

On each SysTick:

```text
increment tick
    -> advance timeout list
    -> move expired tasks to ready queues
    -> BLOCKED -> READY
    -> if idle is running, pend PendSV
```

## Periodic delay

`hr_task_delay_until()` computes each release from the previous release tick,
not from the time at which the task finishes its work. This avoids cumulative
period drift.

## Phase boundary

Phase 7 performs the timeout-driven switch required to leave idle. It does not
yet preempt an arbitrary running task when a higher-priority task wakes, and it
does not rotate equal-priority tasks from SysTick. General preemption and round-
robin time slicing are Phase 8.

## Bare-metal SysTick separation

Earlier target examples retain a dedicated bare-metal SysTick IRQ source. Phase
7 links the kernel SysTick handler instead, preventing duplicate handlers and
keeping the two time models explicit.
