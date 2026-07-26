# RTOS Knowledge Coverage

| Knowledge area | Planned phase | Current state |
|---|---:|---|
| Specification and architecture rules | 0 | Implemented |
| Startup, vector table, linker, clock, GPIO, UART | 1 | Implemented |
| Intrusive lists, ready/wait/timeout structures | 2 | Implemented |
| TCB and independent initial task stacks | 3 | Implemented |
| SVC first-task startup and PSP transition | 4 | Implemented |
| PendSV task-to-task context switching | 5 | Implemented |
| Fixed-priority scheduling integration | 6 | Implemented |
| Kernel SysTick, delay, periodic delay, timeout wake | 7 | Implemented |
| General preemption and tick round-robin | 8 | Implemented |
| Queue and blocking IPC | 9 | Implemented |
| Semaphore, mutex, priority inheritance | 10 | Implemented |
| Task suspend/resume | 11 | Implemented |
| Software timers | 12 | Implemented |
| HairEvent events, Active Objects, flat state machines, time events, pub/sub | 13 | Implemented |
| First-fit allocator lab | 14 | Not started |
| Kernel latency benchmarks | 15 | Not started |
| Diagnostics and stabilization | 16 | Not started |
| Tickless idle | After v1.0 | Deferred |
| MPU isolation | Future major version | Deferred |
| SMP/multi-core | Out of v1.x scope | Deferred |

Phase 10 reuses the generic object wait/timeout path for semaphores and mutexes.
Semaphore waiters are priority/FIFO ordered. Mutex ownership is explicit, and
effective priority is recalculated from all held mutexes so restoration remains
correct when a task owns more than one contended mutex.

Phase 11 adds administrative suspension while preserving ready/wait/timeout
invariants. Phase 12 adds static one-shot and periodic timers whose callbacks run
in a dedicated service task rather than SysTick context. Phase 13 adds the
static-first HairEvent layer with deterministic event pools, Active Objects,
flat state machines, time events, and reference-counted publish/subscribe.
