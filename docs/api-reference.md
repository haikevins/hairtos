# HairRTOS v1.0 Public API Reference

| Module | Main APIs | Context and blocking rules |
|---|---|---|
| Kernel | `hr_kernel_init`, `hr_kernel_start`, state/tick/task-count getters | Init/start from boot task context; start does not return on success |
| Task | create/start, yield, delay, delay-until, suspend/resume, metadata | Delay/suspend/resume are task-only; delay may block; create is static |
| Queue | create, send/receive, ISR send/receive, diagnostics getters | Task APIs may block; ISR APIs never block and use no timeout |
| Semaphore | counting/binary create, take/give, ISR give | Take may block; ISR give never blocks |
| Mutex | normal/recursive create, lock/unlock, owner getters | Task-only; lock may block; priority inheritance enabled |
| Timer | create, start/stop/reset/change-period, metadata | Commands are task-only; callbacks run in timer-service task context |
| Context | critical enter/exit, ISR query, yield-from-ISR | Critical sections are short and nest by restoring prior PRIMASK |
| Diagnostics | task snapshot, health check, runtime statistics, panic record | Health check is task-context and non-blocking; fault capture is port-owned |
| Hooks | panic, stack-overflow, assert hooks; `HR_ASSERT` | Hooks must be bounded; panic/fault hooks must not rely on scheduler services |
| HairEvent | event pool, AO post, state machine, time event, pub/sub | Active Objects block on queues; ISR post is non-blocking |

All kernel objects and stacks are supplied by the application. HairRTOS v1.0
does not allocate kernel runtime objects from the Phase 14 allocator lab.
