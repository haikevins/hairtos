# Phase 7 — Task Delay and Timeout

This STM32F103 target example demonstrates the first kernel-owned SysTick time
base and blocking delays.

- `periodic` uses `hr_task_delay_until()` every 500 ticks;
- `heartbeat` uses `hr_task_delay()` every 1000 ticks;
- when both application tasks are BLOCKED, the idle task executes `WFI`;
- SysTick moves expired tasks from the timeout list back to the ready queues;
- a timeout wakes a task immediately when idle is running.

Build and flash:

```bash
make EXAMPLE=07-task-delay-timeout flash
```

Phase 7 does not yet implement general higher-priority preemption or equal-
priority time slicing. Those belong to Phase 8.
