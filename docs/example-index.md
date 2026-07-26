# HairRTOS Example Index

| Phase | Folder | Environment | Status |
|---:|---|---|---|
| 1 | `01-baremetal-foundation` | STM32 target | Complete |
| 2 | `02-kernel-data-structures-host` | Ubuntu host | Complete |
| 3 | `03-static-task-stack` | STM32 target | Complete |
| 4 | `04-start-first-task` | STM32 target | Complete |
| 5 | `05-cooperative-context-switch` | STM32 target | Complete |
| 6 | `06-priority-scheduler` | STM32 target | Complete |
| 7 | `07-task-delay-timeout` | STM32 target | Complete |
| 8 | `08-preemption-round-robin` | STM32 target | Complete |
| 9 | `09-queue-blocking-ipc` | STM32 target | Complete |
| 10 | `10-01-semaphore-from-isr`, `10-02-mutex-priority-inheritance` | STM32 target | Complete |
| 11 | `11-task-suspend-resume` | STM32 target | Complete |
| 12 | `12-software-timer` | STM32 target | Complete |
| 13 | `13-01-event-post` through `13-06-event-driven-demo` | STM32 target | Complete |
| 14 | `14-memory-allocator-lab` | Host + target | Complete |
| 15 | `15-kernel-benchmark` | STM32 target | Complete |
| 16 | `16-diagnostics-stress-stabilization` | Host + target | Placeholder |

The Phase 2 host example is not flashable. Use `make phase2-example`. Run the Phase 14 native lab with `make phase14-lab`; its separate STM32 example is flashable. Phase 15 is target-only and runs with `make EXAMPLE=15-kernel-benchmark flash`.
Implemented target examples must be selected in the same invocation as flash.
