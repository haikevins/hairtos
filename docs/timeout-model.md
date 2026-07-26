# Timeout Model

`hr_tick_t` is an unsigned 32-bit counter incremented at 1 kHz by the kernel
`SysTick_Handler`.

Phase 7 implements finite task delays:

- `hr_task_delay(ticks)` blocks relative to the current tick;
- `hr_task_delay_until(anchor, period)` blocks to an absolute periodic release;
- `ticks == 0` behaves as a cooperative yield;
- `HR_WAIT_FOREVER` is not accepted by the task-delay API and is reserved for
  future object waits.

## Two-epoch timeout lists

The timeout module maintains a current list and an overflow list. A deadline
whose unsigned addition wraps below the current tick is inserted into the
overflow list. When the kernel tick wraps, the list roles are swapped.

Each list is ordered by wake tick and preserves FIFO order for equal deadlines.

## State transition

```text
RUNNING
  -> remove ready node
  -> add timeout node
  -> BLOCKED
  -> timeout expires
  -> add ready node
  -> READY
```

A timeout node has one owner task, and a task cannot be readied twice.

Periodic work uses `hr_task_delay_until()` so execution time does not accumulate
as phase drift. Periods are bounded to `INT32_MAX` ticks to keep wrap-safe
signed-difference comparisons unambiguous.
