# Source map

| Subsystem | Public | Internal/implementation |
|---|---|---|
| Kernel lifecycle | `hr_kernel.h` | `hr_kernel.c`, `hr_kernel_internal.h` |
| Task | `hr_task.h` | `hr_task.c`, `hr_task_internal.h` |
| Context | `hr_context.h` | `hr_context.c`, arch port |
| Time | `hr_time.h` | `hr_time.c`, `hr_timeout.c` |
| Queue | `hr_queue.h` | `hr_queue.c`, internal header |
| Semaphore | `hr_semaphore.h` | `hr_semaphore.c` |
| Mutex | `hr_mutex.h` | `hr_mutex.c` |
| Timer | `hr_timer.h` | `hr_timer.c` |
| Diagnostics | `hr_diagnostics.h`, `hr_hooks.h` | `hr_diagnostics.c`, fault port |
| Scheduler | không public trực tiếp | `hr_scheduler.c` |
| Intrusive list | không public | `hr_list.c` |
| Wait | không public | `hr_wait.c` |
| Event | `he_event.h` | `he_event.c`, `he_internal.h` |
| FSM | `he_state_machine.h` | `he_state_machine.c` |
| AO | `he_active.h` | `he_active.c` |
| Time Event | `he_time_event.h` | `he_time_event.c` |
| Pub/Sub | `he_pubsub.h` | `he_pubsub.c` |
| Cortex-M3 port | `hr_port.h` internal platform contract | `arch/arm/cortex-m3/*` |
| STM32F1 | target internal | `soc/stm32f1/*`, `drivers/stm32f1/*` |
| Blue Pill | `board.h` | `boards/bluepill_f103c8/*` |
| Target manifest | build contract | `cmake/targets/*` |
| Benchmark | `hr_benchmark.h` | stats + target clock backend |
| Allocator lab | lab headers | `labs/memory-allocator/src` |
