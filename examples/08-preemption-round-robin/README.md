# `08-preemption-round-robin` — Preemption and Round-Robin

> **Environment:** Target  
> **Source:** `examples/08-preemption-round-robin/main.c`  
> **Focus:** Preemption + tick time slicing

[← Root README](../../README.md)

## Table of Contents

- [Objective and Core Concept](#objective)
- [Build Graph and Configuration](#build-graph)
- [Runtime Flow](#runtime)
- [API and Ownership](#api)
- [Invariant / PASS criteria](#pass)
- [Debugging and Failure Modes](#debug)
- [Validation](#validation)
- [Source Map and References](#source-map)

<a id="objective"></a>
## Objective and Core Concept

Two CPU-bound workers that never yield still share CPU time; a higher-priority monitor wakes periodically and preempts them.


<a id="build-graph"></a>
## Build Graph and Configuration

- CMake declares this example as a **Target** environment.
- Modules linked for this example: `platform`, `task_kernel`, `kernel_runtime`, `kernel_time`.
- Reference target: `bluepill_f103c8` — STM32F103C8T6 / Cortex-M3 / nominal 72 MHz / USART1 115200 / active-low PC13 LED.

### Compile-time / source constants

| Symbol | Value in `main.c` |
| --- | --- |
| `MONITOR_TASK_PRIORITY` | `1U` |
| `WORKER_TASK_PRIORITY` | `3U` |
| `TASK_STACK_WORDS` | `192U` |
| `MONITOR_PERIOD_TICKS` | `250U` |

### CMake feature overrides

- The example uses the default configuration except for modules/definitions explicitly declared in `cmake/hairtos_examples.cmake`.

<a id="runtime"></a>
## Runtime Flow

```mermaid
flowchart TB
    WAKE["Task becomes READY"] --> INSERT["Enqueue at priority"]
    INSERT --> BITMAP["Set ready bitmap"]
    BITMAP --> SELECT["Pick highest priority"]
    SELECT --> FRONT["Select FIFO head"]
    FRONT --> RUN["RUNNING"]
    RUN -->|"yield / slice"| ROTATE["Rotate queue"]
    RUN -->|"block"| REMOVE["Remove from ready set"]
    ROTATE --> SELECT
    REMOVE --> SELECT
```


### Details Observed Directly in the Example

- Demonstrate preemption when a higher-priority task becomes READY.
- Demonstrate time slicing between two equal-priority tasks.
- Detect starvation by comparing worker counters.
- Keep PendSV as the only context save/restore path.
- SysTick decides PREEMPT or TIME_SLICE and then pends PendSV.
- Monitor priority 1 is higher than worker priority 3.
- Workers do not call kernel APIs inside their loops.
- Round-robin uses quantum `HR_CFG_TIME_SLICE_TICKS`.
- `hairtos/hr_time.h`
- `hr_port.h`
- `hr_task_delay_until()`
- `hr_task_current()`
- `task_kernel`
- `kernel_runtime`
- `kernel_time`
- After initial activation, both worker counters increase between consecutive reports.
- The monitor runs approximately every 250 ticks.
- No starvation error appears.
- Hardware — STM32F103C8T6 Blue Pill — Runs the target firmware.
- Flash/debug — ST-Link V2 over SWD — OpenOCD is used to flash, verify, and reset the target.
- UART — USART1, PA9 TX / PA10 RX, 115200 8-N-1 — Observes logs and PASS/FAIL status.
- LED — PC13, active-low — Displays heartbeat or observable status.
- `monitor` — Priority 1, stack 192 words — Runs every 250 ticks and checks counters.
- `worker-a` — Priority 3, stack 192 words — CPU-bound counter.

<a id="api"></a>
## API and Ownership

APIs called directly from `main.c` (extracted from source):

- `board_init()`
- `board_led_toggle()`
- `board_panic()`
- `board_uart_write_line()`
- `board_uart_write_string()`
- `board_uart_write_u32()`
- `hr_kernel_init()`
- `hr_kernel_start()`
- `hr_port_thread_uses_psp()`
- `hr_task_create_static()`
- `hr_task_current()`
- `hr_task_delay_until()`
- `hr_task_start()`
- `hr_time_now()`

Ownership rules to keep in mind:

- `hr_task_t`, stacks, queue/semaphore/mutex/timer objects, and haievent storage in the examples are all static/caller-owned.
- Kernel APIs retain pointers to this storage after creation, so the storage lifetime must cover the entire period in which the object remains active.
- ISR paths must not call blocking APIs. `_from_isr` APIs perform bounded work and return `higher_priority_task_woken` so PendSV can perform any required switch after ISR exit.
- Dynamic haievent events allocated from a pool use retain/release semantics; static events are not freed automatically by the framework.

<a id="pass"></a>
## Invariants and PASS Criteria

- A READY task appears exactly once in the ready set; its node must not simultaneously belong to another list.
- Selection is independent of registration order across different priorities: the lowest-numbered priority whose bitmap bit is set always wins.
- Tasks at the same priority are ordered FIFO; `yield`/time slicing rotates the highest-priority ready queue rather than changing priority.
- Preemption occurs only when a READY task has a numerically lower effective priority than the current task; equal-priority peers require yield or time slicing to rotate execution.
- Any change to the effective priority of a READY task must requeue its ready node so the bitmap/list reflects the new priority.

Hard-coded checks/logs in the source:

- `ERROR: invalid task context.`
- `ERROR: equal-priority worker starvation detected.`
- `ERROR: monitor delay failed.`
- `Kernel initialization failed.`
- `Monitor task creation failed.`
- `Worker A creation failed.`
- `Worker B creation failed.`
- `Task registration failed.`
- `ERROR: hr_kernel_start returned status=`

<a id="debug"></a>
## Debugging and Failure Modes

- A higher-priority task wakes but does not preempt: inspect whether the wake path pends PendSV.
- Equal-priority tasks do not round-robin: inspect the time-slice counter and FIFO rotation.
- Incorrect priority ordering: inspect effective priority and the ready bitmap.
- If the issue appears only after many ticks, also inspect time-slice reset behavior when task/block state changes.

<a id="validation"></a>
## Validation

- This example is target-only in CMake; host evidence does not replace ARM cross-build, OpenOCD, and hardware validation.
- Host validation baseline: `make TARGET=bluepill_f103c8 host-tests` passes the entire suite.

### Standard Commands

```bash
make TARGET=bluepill_f103c8 EXAMPLE=08-preemption-round-robin build
make TARGET=bluepill_f103c8 EXAMPLE=08-preemption-round-robin run
make TARGET=bluepill_f103c8 EXAMPLE=08-preemption-round-robin check
```

<a id="source-map"></a>
## Source Map and References

- `examples/08-preemption-round-robin/main.c`
- `cmake/hairtos_examples.cmake`
- `kernel/src/hr_scheduler.c`
- `kernel/internal/hr_scheduler_internal.h`
- `kernel/src/hr_kernel.c`
- `tests/host/test_ready_queue.c`
- `tests/host/test_scheduler_policy.c`
- `labs/memory-allocator/`

### References


**Implementation sources in the repository:**
- `kernel/src/hr_scheduler.c`
- `kernel/internal/hr_scheduler_internal.h`
- `kernel/src/hr_kernel.c`
- `tests/host/test_ready_queue.c`
- `tests/host/test_scheduler_policy.c`
- `labs/memory-allocator/`
