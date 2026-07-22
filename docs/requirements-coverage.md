# RTOS Knowledge Coverage

This matrix maps HairRTOS phases to the major topics expected from a
self-developed RTOS project.

| Knowledge area | Planned phase | Phase 1 state |
|---|---:|---|
| Startup, vector table, linker, memory sections | 1 | Implemented |
| Clock, GPIO, UART, hardware time base | 1 | Implemented |
| Intrusive kernel lists | 2 | Specification only |
| TCB and independent stacks | 3 | Specification only |
| SVC first-task startup | 4 | Specification only |
| PendSV context switching | 5 | Specification only |
| Priority scheduling | 6 | Specification only |
| Delay and timeout | 7 | Specification only |
| Preemption and round-robin | 8 | Specification only |
| Queue and blocking IPC | 9 | Specification only |
| Semaphore and mutex | 10 | Specification only |
| Priority inheritance | 10 | Specification only |
| Task suspend/resume | 11 | Added to roadmap |
| Software timers | 12 | Specification only |
| Event, Active Object, state machine | 13 | Specification only |
| Dynamic event pools | 13 | Specification only |
| First-fit allocator, split, coalesce | 14 | Added as isolated lab |
| Context-switch and wake-up benchmarks | 15 | Added to roadmap |
| Stack guards and runtime diagnostics | 16 | Specification only |
| Tickless idle | After v1.0 | Deferred |
| MPU isolation | Future major version | Deferred |
| SMP/multi-core | Out of v1.x scope | Deferred |

## Important interpretation

Phase 1 proves only the hardware and build foundation. Placeholder headers,
source files, examples, labs, and benchmark directories do not mean those RTOS
features have been implemented.
