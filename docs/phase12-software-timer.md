# Phase 12 — Software Timer Service

## Implemented scope

Phase 12 adds static one-shot and auto-reload timers, an ordered expiration
engine, pending-expiration accounting, and a dedicated timer-service task.
Callbacks never execute in SysTick context.

## Public API

```c
hr_timer_create_static(...);
hr_timer_start(...);
hr_timer_stop(...);
hr_timer_reset(...);
hr_timer_change_period(...);
hr_timer_is_active(...);
```

## Scheduling policy

The first timer creation lazily registers the timer-service task. The Phase 12
target build sets its priority to 1. SysTick wakes it through a binary semaphore;
PendSV performs the actual preemption and context switch.

## Validation

Host tests cover one-shot expiry, periodic rearm, equal-deadline FIFO order,
reset, stop, period change, pending callbacks, invalid state, and ISR-context
rejection. Target validation checks strong SysTick/PendSV handlers and timer API
symbols. Physical callback jitter validation remains a board-side task.
