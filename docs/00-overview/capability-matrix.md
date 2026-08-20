# Capability matrix — `hairtos 1.0.0-rc1`

> **Quy ước:** **Có** = source + test/example/evidence hiện diện; **Một phần** = có nền tảng nhưng chưa đủ portability/coverage; **Chưa** = không implemented; **Không chủ đích** = nằm ngoài baseline v1.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](architecture.md) · [Next →](coding-standard.md)

## Mục lục

- [Kernel/task](#kernel)
- [Time/scheduler](#time)
- [IPC/synchronization](#ipc)
- [`haievent`](#event)
- [Diagnostics/benchmark](#diag)
- [Platform/tooling](#platform)
- [Not implemented](#missing)
- [Evidence](#evidence)

<a id="kernel"></a>
## Kernel / task

| Capability | Trạng thái | Implementation/evidence |
| --- | --- | --- |
| Static task creation | Có | caller-owned `hr_task_t` + stack; `hr_task_create_static()` |
| Dynamic task creation | Không chủ đích | `HR_CFG_DYNAMIC_ALLOCATION=0` |
| Initial Cortex-M stack | Có | `hr_port_initialize_stack_raw()` + host tests |
| Start first task | Có | SVC path in `hr_portasm.S` |
| Context switch | Có | PendSV save/restore R4–R11 + hardware frame |
| Task states | Có | CREATED/READY/RUNNING/BLOCKED/SUSPENDED |
| Stack guard | Có | `0xDEADBEEF` |
| Stack fill/high-watermark | Có | fill `0xA5`; task diagnostics |
| Task return handling | Có baseline | initial LR → `hr_task_exit_error()`; production recovery policy chưa có |
| Idle task | Có | kernel-owned static idle task, WFI path |

<a id="time"></a>
## Scheduler / time

| Capability | Trạng thái | Implementation/evidence |
| --- | --- | --- |
| Fixed-priority scheduler | Có | 8 FIFO ready queues + bitmap |
| Priority convention | Có | số nhỏ hơn = cao hơn |
| Preemption | Có | strict higher effective priority |
| Equal-priority round-robin | Có | configurable tick slice, default 1 tick |
| Cooperative yield | Có | `hr_task_yield()` |
| Relative delay | Có | `hr_task_delay()` |
| Periodic absolute delay | Có | `hr_task_delay_until()` |
| Finite timeout | Có | wait + timeout node |
| Infinite wait | Có | `HR_WAIT_FOREVER` |
| Tick wrap handling | Có | current/overflow timeout lists + tests |
| Tickless idle | Chưa | Version 2 roadmap |
| SMP scheduler | Không chủ đích | `HR_CFG_SINGLE_CORE=1` |

<a id="ipc"></a>
## IPC / synchronization

| Capability | Trạng thái | Ghi chú |
| --- | --- | --- |
| Static FIFO queue | Có | circular buffer + caller storage |
| Blocking send/receive | Có | timeout + priority waiters |
| Direct queue handoff | Có | sender↔receiver waiter fast path |
| Queue ISR send/receive | Có | non-blocking + wake flag |
| Counting semaphore | Có | count/max_count + waiter list |
| Binary semaphore | Có | max count 1 |
| Semaphore give from ISR | Có | non-blocking |
| Non-recursive mutex | Có | owner + waiters |
| Recursive mutex | Có | recursion count |
| Priority inheritance | Có | recompute effective priority |
| Chained inheritance | Có | propagation theo owned/waited mutex path |
| Automatic deadlock prevention | Chưa | không có wait-for graph/deadlock detector |
| Suspend/resume | Có | READY/RUNNING/BLOCKED-aware administrative control |
| Software timer | Có | timer-service task; callback in task context |

<a id="event"></a>
## `haievent`

| Capability | Trạng thái | Ghi chú |
| --- | --- | --- |
| Static event | Có | caller-owned |
| Fixed-block dynamic event pool | Có | no general heap |
| Reference counting | Có | uint16_t with overflow guard |
| Flat FSM | Có | ENTRY/EXIT/INIT + transition |
| Initial transition bound | Có | max 8 |
| Active Object | Có | one task + one queue + FSM |
| Task post | Có | retains dynamic event |
| ISR post | Có | non-blocking path |
| Time Event | Có | software timer adapter |
| Publish/subscribe | Có | static subscriber matrix + snapshot publish |
| Hierarchical State Machine | Chưa | Version 2 roadmap |
| Deferred event/recall | Chưa | Version 2 roadmap |
| History state | Chưa | ngoài baseline 2.0 core ban đầu |
| Shared AO executor | Chưa | one-task-per-AO là v1 baseline |

<a id="diag"></a>
## Diagnostics / benchmark

| Capability | Trạng thái | Ghi chú |
| --- | --- | --- |
| Runtime counters | Có | compile-time enable |
| Kernel invariant validation | Có | internal full-structure check |
| Per-task stack diagnostics | Có | guard + free/used words |
| Retained panic record | Có | `.noinit.hairtos` |
| Cortex-M fault context | Có target | stacked registers + SCB fault status |
| Weak hooks | Có | panic/stack/assert hooks |
| DWT benchmark clock | Có target | Blue Pill/Cortex-M3 |
| Statistical benchmark | Có | bounded samples + min/p50/mean/p95/max |
| External marker | Có target | PB0 active-high |
| Fixed-size runtime trace ring | Chưa | Version 2 roadmap |

<a id="platform"></a>
## Platform / tooling

| Capability | Trạng thái | Ghi chú |
| --- | --- | --- |
| CMake target manifest | Có | one manifest per target |
| Host environment | Có | GCC + ASan/UBSan |
| GCC ARM toolchain support | Có trong config | environment audit hiện không cài cross compiler |
| Clang ARM toolchain support | Có trong config | compile path defined; target evidence cần toolchain |
| Blue Pill target | Có | complete binding |
| Second hardware target | Chưa | Version 2 success criterion |
| OpenOCD config | Có | ST-Link/Blue Pill |
| GDB helper | Có | `tools/gdb/hairtos.gdb` |
| Compile database | Có | CMake export + Makefile symlink |

<a id="missing"></a>
## Không có trong v1

- FPU context switching;
- MPU task isolation;
- SMP/multicore;
- tickless idle;
- general POSIX layer;
- filesystem/network stack;
- general dynamic kernel heap;
- HSM/deferred/history;
- automatic deadlock prevention;
- safety certification.

<a id="evidence"></a>
## Evidence

Audit hiện tại đã chạy `make TARGET=bluepill_f103c8 host-tests` và toàn bộ suite PASS. Ba host-capable example 02/14/16 cũng chạy PASS; scheduler stress đạt 500.000 iteration. Capability phụ thuộc hardware/assembly vẫn cần target validation để gọi là runtime-proven.

## References

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)
- [ST RM0008 — STM32F10x Reference Manual](https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
- [STM32F103 documentation portal](https://www.st.com/en/microcontrollers-microprocessors/stm32f103/documentation.html)

**Source:** `config/hairtos_config.h`, `config/haievent_config.h`, `kernel/`, `haievent/`, `cmake/hairtos_examples.cmake`, `cmake/targets/bluepill_f103c8.cmake`, `tests/`.
