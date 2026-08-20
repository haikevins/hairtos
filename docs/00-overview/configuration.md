# Compile-time configuration

> **Scope:** Actual macros in `config/hairtos_config.h` and `config/haievent_config.h`, plus compile-definition overrides from CMake examples.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](coding-standard.md) · [Next →](dependency-rules.md)

## Table of Contents

- [Kernel configuration](#kernel)
- [`haievent` configuration](#event)
- [Opaque storage sizing](#storage)
- [Per-example overrides](#overrides)
- [Configuration invariants](#invariants)
- [Validation](#validation)

<a id="kernel"></a>
## Kernel configuration

| Macro | Default | Implementation Meaning |
| --- | ---: | --- |
| `HR_CFG_TICK_RATE_HZ` | 1000 | nominal kernel tick contract |
| `HR_CFG_PRIORITY_COUNT` | 8 | number of ready queues / priority bits used in the bitmap |
| `HR_CFG_IDLE_PRIORITY` | 7 | idle priority; user effective priorities cannot enter the idle range in the internal setter |
| `HR_CFG_MAX_TASKS` | 8 | registry size |
| `HR_CFG_PREEMPTION` | 1 | a strictly higher-priority wakeup may request PendSV |
| `HR_CFG_TIME_SLICING` | 1 | equal-priority tick rotation |
| `HR_CFG_TIME_SLICE_TICKS` | 1 | default quantum |
| `HR_CFG_STATIC_ALLOCATION` | 1 | design contract |
| `HR_CFG_DYNAMIC_ALLOCATION` | 0 | no kernel heap |
| Queue / semaphore / mutex | 1 | compile feature enable |
| `HR_CFG_ENABLE_SOFTWARE_TIMER` | 1 | may be overridden by an example |
| `HR_CFG_ENABLE_ASSERT` | 1 | assert hook/path |
| `HR_CFG_ENABLE_STACK_CHECK` | 1 | guard/high-watermark support |
| `HR_CFG_ENABLE_RUNTIME_STATS` | 0 | off by default; enabled by example 16 |
| `HR_CFG_ENABLE_DIAGNOSTICS` | 0 | off by default; enabled by example 16 |
| `HR_CFG_DIAGNOSTICS_STACK_MARGIN_WORDS` | 16 | low-stack health threshold |
| `HR_CFG_IDLE_STACK_WORDS` | 128 | idle stack |
| `HR_CFG_TIMER_TASK_PRIORITY` | 6 default | overridden to 1 in timer/event/benchmark/diagnostics examples |
| `HR_CFG_TIMER_TASK_STACK_WORDS` | 160 | timer service stack |
| `HR_CFG_MIN_TASK_STACK_WORDS` | 32 | create-time minimum |
| `HR_CFG_USE_FPU` | 0 | no FPU context |
| `HR_CFG_USE_MPU` | 0 | no MPU isolation |
| `HR_CFG_SINGLE_CORE` | 1 | no SMP |
| `HR_WAIT_FOREVER` | `0xFFFFFFFF` | infinite wait sentinel |
| `HR_NO_WAIT` | 0 | immediate/nonblocking sentinel |

<a id="event"></a>
## `haievent` configuration

| Macro | Default | Meaning |
| --- | ---: | --- |
| `HE_CFG_ENABLED` | 1 | framework compiled when module selected |
| Active Object | 1 | v1 AO support |
| Flat FSM | 1 | current state machine model |
| HSM | 0 | not implemented v1 |
| Time Event | 1 | software timer adapter |
| Event pool | 1 | fixed-block dynamic event |
| Publish/Subscribe | 1 | static subscriber table |
| `HE_CFG_MAX_ACTIVE_OBJECTS` | 8 | pubsub snapshot/subscriber bound |
| `HE_CFG_MAX_SIGNALS` | 64 | table signal bound |
| `HE_CFG_MAX_INIT_TRANSITIONS` | 8 | protect init-transition loops |

<a id="storage"></a>
## Opaque storage sizing

Public object bytes are compile-time ABI-within-one-build contracts:

| Public object | Storage bytes |
| --- | ---: |
| `hr_task_t` | 384 |
| `hr_queue_t` | 192 |
| `hr_semaphore_t` | 96 |
| `hr_mutex_t` | 160 |
| `hr_timer_t` | 160 |
| event pool | 64 |
| state machine | 64 |
| Active Object | 768 |
| Time Event | 256 |
| pub/sub | 64 |

Internal headers contain `_Static_assert(sizeof(control_block) <= sizeof(public_object))`; reducing a storage macro too far causes a compile-time failure rather than silent overwrite.

<a id="overrides"></a>
## Per-example overrides

`cmake/hairtos_examples.cmake` is the actual source of example-specific overrides:

- Example 07: `PREEMPTION=0`, `TIME_SLICING=0`.
- Example 12 and all 13-* examples enable software timers with timer-task priority 1.
- Example 15: preemption on, time slicing off, timer priority 1.
- Example 16: preemption/time slicing/software timer + diagnostics + runtime stats on, timer priority 1.
- Many early examples intentionally set `HR_CFG_ENABLE_SOFTWARE_TIMER=0` to exclude modules/behavior that have not yet been introduced.

<a id="invariants"></a>
## Configuration invariants

- Priority count must fit the bitmap implementation, and idle priority must remain within range.
- Timer-task priority must not accidentally conflict semantically with idle.
- Changing configuration macros requires rebuilding the entire binary; object storage/layout is not runtime-tunable.
- A disabled feature must expose a clear stub/status path rather than leaving behavior half-enabled.
- Target-specific compile definitions live in the manifest; example-specific definitions live in the examples map.

<a id="validation"></a>
## Validation

`config/config_check.h` and compile-time assertions catch some invalid combinations. Host tests exercise multiple feature paths; CMake composes example-specific overrides in each build tree.

## References

- [CMake — CMAKE_TOOLCHAIN_FILE](https://cmake.org/cmake/help/latest/variable/CMAKE_TOOLCHAIN_FILE.html)
- [CMake — CMAKE_EXPORT_COMPILE_COMMANDS](https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html)

**Source:** `config/hairtos_config.h`, `config/haievent_config.h`, `config/config_check.h`, `cmake/hairtos_examples.cmake`.
