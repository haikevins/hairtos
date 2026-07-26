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
| General preemption and tick round-robin | 8 | Not started |
| Queue and blocking IPC | 9 | Not started |
| Semaphore, mutex, priority inheritance | 10 | Not started |
| Task suspend/resume | 11 | Not started |
| Software timers | 12 | Not started |
| HairEvent Active Objects and state machines | 13 | Not started |
| First-fit allocator lab | 14 | Not started |
| Kernel latency benchmarks | 15 | Not started |
| Diagnostics and stabilization | 16 | Not started |
| Tickless idle | After v1.0 | Deferred |
| MPU isolation | Future major version | Deferred |
| SMP/multi-core | Out of v1.x scope | Deferred |

Phase 7 proves blocking delay and timeout wake-up. A woken task immediately leaves
idle through PendSV, but preemption of an arbitrary running lower-priority task
and tick-based round-robin remain Phase 8 work.
