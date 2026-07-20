# Timeout Model

`hr_tick_t` is an unsigned 32-bit counter incremented at 1 kHz.

- `HR_NO_WAIT` is non-blocking.
- Finite values are maximum wait ticks.
- `HR_WAIT_FOREVER` has no timeout.

A finite blocked task belongs to both an object wait list and a timeout
structure. Object availability and timeout race through one atomic transition,
so a task cannot be readied twice.

Tick wrap is a required test case. Tests set the tick counter near `UINT32_MAX`.
The implementation will choose either bounded signed-difference comparisons or
two ordered timeout lists before Phase 8.

Periodic work uses `hr_task_delay_until()` to avoid accumulating execution-time
drift.
