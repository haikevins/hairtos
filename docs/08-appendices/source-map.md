# Source map

> Use this file as the fastest path from a concept/API to its implementation/test.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](known-limitations.md)

| Subsystem | Public surface | Internal/implementation | Tests/evidence |
| --- | --- | --- | --- |
| Kernel lifecycle | `hr_kernel.h` | `hr_kernel.c`, `hr_kernel_internal.h` | `test_kernel_start.c` |
| Task | `hr_task.h` | `hr_task.c`, `hr_task_internal.h` | `test_task.c` |
| Context | `hr_context.h` | `hr_context.c`, Cortex-M port | mock/target examples |
| Scheduler | indirect through task/kernel | `hr_scheduler.c` | `test_ready_queue.c`, `test_scheduler_policy.c`, stress |
| Wait | internal | `hr_wait.c` | `test_wait_list.c` |
| Timeout | internal | `hr_timeout.c` | `test_timeout.c` |
| Queue | `hr_queue.h` | `hr_queue.c` | `test_queue.c` |
| Semaphore | `hr_semaphore.h` | `hr_semaphore.c` | `test_semaphore.c` |
| Mutex | `hr_mutex.h` | `hr_mutex.c` | `test_mutex.c` |
| Timer | `hr_timer.h` | `hr_timer.c` | `test_timer.c` |
| Diagnostics | `hr_diagnostics.h`, `hr_hooks.h` | `hr_diagnostics.c`, fault port | `test_diagnostics.c`, ex16 |
| Event | `he_event.h` | `he_event.c` | `test_haievent.c`, 13-* |
| FSM | `he_state_machine.h` | `he_state_machine.c` | `test_haievent.c`, 13-03 |
| Active Object | `he_active.h` | `he_active.c` | `test_haievent.c`, 13-02/06 |
| Time Event | `he_time_event.h` | `he_time_event.c` | 13-04/06 |
| Pub/Sub | `he_pubsub.h` | `he_pubsub.c` | `test_haievent.c`, 13-05/06 |
| Cortex-M port | `hr_port*.h` target include | `arch/arm/cortex-m3/*` | stack tests + target examples |
| SoC | target headers | `soc/stm32f1/*` | target runtime |
| Board | `board.h` | `boards/bluepill_f103c8/*` | all target examples |
| Drivers | `drivers/include/*` | `drivers/stm32f1/*` | target runtime |
| Allocator lab | lab headers | `labs/memory-allocator/src/*` | allocator tests + ex14 |
| Benchmark | `hr_benchmark.h` | stats + DWT clock | test_benchmark + ex15 |

## Debug lookup rule

- Wrong task selected → scheduler/ready set + effective priority.
- Task never wakes → object wait list + timeout node + cleanup result.
- Crash on switch → initial stack/TCB saved SP/PendSV/SVC.
- Event leak → retain/release path + pubsub ownership.
- Timer callback in wrong context → timer service path.
- Fault lost after reset → linker `.noinit` + diagnostics signature/version.
