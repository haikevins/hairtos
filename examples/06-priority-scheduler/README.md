# Phase 6 — Priority Scheduler

This STM32 target example validates the fixed-priority scheduler policy.

## Scenario

Tasks are registered in this order:

1. `low`, priority 5;
2. `high-a`, priority 1;
3. `high-b`, priority 1.

Smaller priority numbers are more urgent. The scheduler must start `high-a`
even though `low` was registered first. Cooperative yield rotates only the
priority-1 FIFO queue:

```text
high-a -> high-b -> high-a -> high-b
```

The priority-5 task remains READY and intentionally panics if it ever executes
while the two priority-1 tasks remain READY.

## Build and flash

```bash
make EXAMPLE=06-priority-scheduler build
make EXAMPLE=06-priority-scheduler run
```

## Expected UART pattern

```text
hairtos Phase 6
Fixed-priority scheduler: smaller number means higher priority.
Low task is registered first but must never run while high tasks are READY.
Starting scheduler through SVC...
selected=high-A priority=1 counter=1 -> yield to equal-priority peer
selected=high-B priority=1 counter=1010 -> yield to equal-priority peer
selected=high-A priority=1 counter=2 -> yield to equal-priority peer
```

Phase 6 is still cooperative. A higher-priority task becoming READY at runtime
does not yet trigger automatic preemption; that behavior is introduced in
Phase 8 after kernel delay/blocking exists.
