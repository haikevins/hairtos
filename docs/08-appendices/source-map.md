# Source map

| Subsystem | Public header | Internal/implementation |
|---|---|---|
| Kernel lifecycle | `hr_kernel.h` | `hr_kernel.c`, `hr_kernel_internal.h` |
| Task | `hr_task.h` | `hr_task.c`, `hr_task_internal.h` |
| Time | `hr_time.h` | `hr_time.c`, `hr_timeout.c` |
| Queue | `hr_queue.h` | `hr_queue.c`, `hr_queue_internal.h` |
| Semaphore | `hr_semaphore.h` | `hr_semaphore.c`, internal header |
| Mutex | `hr_mutex.h` | `hr_mutex.c`, internal header |
| Timer | `hr_timer.h` | `hr_timer.c`, internal header |
| Diagnostics | `hr_diagnostics.h`, `hr_hooks.h` | `hr_diagnostics.c`, fault port |
| Context wrapper | `hr_context.h` | `hr_context.c`, Cortex-M3 port |
| haievent | `haievent/*.h` | `haievent/src`, `he_internal.h` |
| Allocator lab | `hr_heap_lab.h`, `hr_pool_lab.h` | `labs/memory-allocator/src` |
| Benchmark | `hr_benchmark.h` | `benchmarks/kernel/src` |

## Build source of truth

Target source selection nằm trong `Makefile` và `CMakeLists.txt`. Host tests nằm trong `tests/host`; stress trong `tests/stress`; validation trong `tools/scripts`.
