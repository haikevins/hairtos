# Capability matrix — `hairtos 1.0.0-rc1`

> **Convention:** **Yes** = source + test/example/evidence exists; **Partial** = foundation exists but portability/coverage is incomplete; **Not yet** = not implemented; **Intentionally out of scope** = outside the v1 baseline.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](architecture.md) · [Next →](coding-standard.md)

## Table of Contents

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

| Capability | Status | Implementation/evidence |
| --- | --- | --- |
| Static task creation | Yes | caller-owned `hr_task_t` + stack; `hr_task_create_static()` |
| Dynamic task creation | Intentionally out of scope | `HR_CFG_DYNAMIC_ALLOCATION=0` |
| Initial Cortex-M stack | Yes | `hr_port_initialize_stack_raw()` + host tests |
| Start first task | Yes | SVC path in `hr_portasm.S` |
| Context switch | Yes | PendSV save/restore R4–R11 + hardware frame |
| Task states | Yes | CREATED/READY/RUNNING/BLOCKED/SUSPENDED |
| Stack guard | Yes | `0xDEADBEEF` |
| Stack fill/high-water mark | Yes | fill `0xA5`; task diagnostics |
| Task return handling | Baseline present | initial LR → `hr_task_exit_error()`; no production recovery policy yet |
| Idle task | Yes | kernel-owned static idle task, WFI path |

<a id="time"></a>
## Scheduler / time

| Capability | Status | Implementation/evidence |
| --- | --- | --- |
| Fixed-priority scheduler | Yes | 8 FIFO ready queues + bitmap |
| Priority convention | Yes | lower number = higher priority |
| Preemption | Yes | strictly higher effective priority |
| Equal-priority round-robin | Yes | configurable tick slice, default 1 tick |
| Cooperative yield | Yes | `hr_task_yield()` |
| Relative delay | Yes | `hr_task_delay()` |
| Periodic absolute delay | Yes | `hr_task_delay_until()` |
| Finite timeout | Yes | wait + timeout node |
| Infinite wait | Yes | `HR_WAIT_FOREVER` |
| Tick wrap handling | Yes | current/overflow timeout lists + tests |
| Tickless idle | Not yet | Version 2 roadmap |
| SMP scheduler | Intentionally out of scope | `HR_CFG_SINGLE_CORE=1` |

<a id="ipc"></a>
## IPC / synchronization

| Capability | Status | Notes |
| --- | --- | --- |
| Static FIFO queue | Yes | circular buffer + caller storage |
| Blocking send/receive | Yes | timeout + priority waiters |
| Direct queue handoff | Yes | sender↔receiver waiter fast path |
| Queue ISR send/receive | Yes | non-blocking + wake flag |
| Counting semaphore | Yes | count/max_count + waiter list |
| Binary semaphore | Yes | max count 1 |
| Semaphore give from ISR | Yes | non-blocking |
| Non-recursive mutex | Yes | owner + waiters |
| Recursive mutex | Yes | recursion count |
| Priority inheritance | Yes | recompute effective priority |
| Chained inheritance | Yes | propagation along owned/waited mutex path |
| Automatic deadlock prevention | Not yet | no wait-for graph/deadlock detector |
| Suspend/resume | Yes | READY/RUNNING/BLOCKED-aware administrative control |
| Software timer | Yes | timer-service task; callback in task context |

<a id="event"></a>
## `haievent`

| Capability | Status | Notes |
| --- | --- | --- |
| Static event | Yes | caller-owned |
| Fixed-block dynamic event pool | Yes | no general heap |
| Reference counting | Yes | uint16_t with overflow guard |
| Flat FSM | Yes | ENTRY/EXIT/INIT + transition |
| Initial transition bound | Yes | max 8 |
| Active Object | Yes | one task + one queue + FSM |
| Task post | Yes | retains dynamic event |
| ISR post | Yes | non-blocking path |
| Time Event | Yes | software timer adapter |
| Publish/subscribe | Yes | static subscriber matrix + snapshot publish |
| Hierarchical State Machine | Not yet | Version 2 roadmap |
| Deferred event/recall | Not yet | Version 2 roadmap |
| History state | Not yet | outside the initial 2.0 core baseline |
| Shared AO executor | Not yet | one-task-per-AO is the v1 baseline |

<a id="diag"></a>
## Diagnostics / benchmark

| Capability | Status | Notes |
| --- | --- | --- |
| Runtime counters | Yes | compile-time enable |
| Kernel invariant validation | Yes | internal full-structure check |
| Per-task stack diagnostics | Yes | guard + free/used words |
| Retained panic record | Yes | `.noinit.hairtos` |
| Cortex-M fault context | Target capability | stacked registers + SCB fault status |
| Weak hooks | Yes | panic/stack/assert hooks |
| DWT benchmark clock | Target capability | Blue Pill/Cortex-M3 |
| Statistical benchmark | Yes | bounded samples + min/p50/mean/p95/max |
| External marker | Target capability | PB0 active-high |
| Fixed-size runtime trace ring | Not yet | Version 2 roadmap |

<a id="platform"></a>
## Platform / tooling

| Capability | Status | Notes |
| --- | --- | --- |
| CMake target manifest | Yes | one manifest per target |
| Host environment | Yes | GCC + ASan/UBSan |
| GCC ARM toolchain support | Present in configuration | target validation requires cross compiler |
| Clang ARM toolchain support | Present in configuration | compile path defined; target evidence requires toolchain |
| Blue Pill target | Yes | complete binding |
| Second hardware target | Not yet | Version 2 success criterion |
| OpenOCD configuration | Yes | ST-Link/Blue Pill |
| GDB helper | Yes | `tools/gdb/hairtos.gdb` |
| Compile database | Yes | CMake export + Makefile symlink |

<a id="missing"></a>
## Not Present in v1

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

Validation baseline: `make TARGET=bluepill_f103c8 host-tests` passes the full suite. Host-capable examples 02/14/16 also pass; scheduler stress reaches 500,000 iterations. Capabilities that depend on hardware/assembly still require target validation before being considered runtime-proven.

## References

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)
- [ST RM0008 — STM32F10x Reference Manual](https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
- [STM32F103 documentation portal](https://www.st.com/en/microcontrollers-microprocessors/stm32f103/documentation.html)

**Source:** `config/hairtos_config.h`, `config/haievent_config.h`, `kernel/`, `haievent/`, `cmake/hairtos_examples.cmake`, `cmake/targets/bluepill_f103c8.cmake`, `tests/`.
