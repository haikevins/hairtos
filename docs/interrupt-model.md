# Interrupt Model

ISRs capture minimal hardware information and defer long processing to tasks or
Active Objects.

Every public API is classified by whether it is task-only, ISR-safe, may block,
may schedule, and is valid before kernel start. ISR-safe functions use the
`_from_isr` suffix.

ISRs must not block, delay, lock mutexes, wait for queue space, dispatch state
machines, use a general heap, or call task-context-only APIs.

An ISR-safe service can set `higher_priority_task_woken`. The ISR then calls
`hr_port_yield_from_isr()`, which only pends PendSV.

Phase 0 permits a PRIMASK-based nested critical section for first correctness.
A later Cortex-M3 optimization may use BASEPRI. Critical sections preserve the
previous interrupt state, remain short, never block, and never call user code.

The final port must document which interrupt priorities are permitted to call
kernel APIs.
