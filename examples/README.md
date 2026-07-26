# HairRTOS Examples

Example directory numbers follow the official HairRTOS roadmap phase numbers.
Phase 0 has no runtime example because it contains specifications only.

## Environment legend

| Environment | Meaning |
|---|---|
| **Host** | Native Ubuntu executable built with the system `cc`, GCC, or Clang. It does not use STM32 startup code and cannot be flashed. |
| **Target** | Cortex-M3 firmware cross-compiled for the STM32F103C8T6 Blue Pill. |
| **Host + Target** | The phase is planned to include both native tests/labs and an STM32 demonstration. |

## Example matrix

| Folder | Phase | Environment | Status | How to run now |
|---|---:|---|---|---|
| `01-baremetal-foundation` | 1 | Target | ✅ Implemented | `make EXAMPLE=01-baremetal-foundation flash` |
| `02-kernel-data-structures-host` | 2 | Host | ✅ Implemented | `make phase2-example` |
| `03-static-task-stack` | 3 | Target | ✅ Implemented | `make EXAMPLE=03-static-task-stack flash` |
| `04-start-first-task` | 4 | Target | ✅ Implemented | `make EXAMPLE=04-start-first-task flash` |
| `05-cooperative-context-switch` | 5 | Target | ✅ Implemented | `make EXAMPLE=05-cooperative-context-switch flash` |
| `06-priority-scheduler` | 6 | Target | ✅ Implemented | `make EXAMPLE=06-priority-scheduler flash` |
| `07-task-delay-timeout` | 7 | Target | ✅ Implemented | `make EXAMPLE=07-task-delay-timeout flash` |
| `08-preemption-round-robin` | 8 | Target | ✅ Implemented | `make EXAMPLE=08-preemption-round-robin flash` |
| `09-queue-blocking-ipc` | 9 | Target | ✅ Implemented | `make EXAMPLE=09-queue-blocking-ipc flash` |
| `10-01-semaphore-from-isr` | 10 | Target | ✅ Implemented | `make EXAMPLE=10-01-semaphore-from-isr flash` |
| `10-02-mutex-priority-inheritance` | 10 | Target | ✅ Implemented | `make EXAMPLE=10-02-mutex-priority-inheritance flash` |
| `11-task-suspend-resume` | 11 | Target | ✅ Implemented | `make EXAMPLE=11-task-suspend-resume flash` |
| `12-software-timer` | 12 | Target | ✅ Implemented | `make EXAMPLE=12-software-timer flash` |
| `13-01-event-post` | 13 | Target | ✅ Implemented | `make EXAMPLE=13-01-event-post flash` |
| `13-02-active-object` | 13 | Target | ✅ Implemented | `make EXAMPLE=13-02-active-object flash` |
| `13-03-flat-state-machine` | 13 | Target | ✅ Implemented | `make EXAMPLE=13-03-flat-state-machine flash` |
| `13-04-time-event` | 13 | Target | ✅ Implemented | `make EXAMPLE=13-04-time-event flash` |
| `13-05-publish-subscribe` | 13 | Target | ✅ Implemented | `make EXAMPLE=13-05-publish-subscribe flash` |
| `13-06-event-driven-demo` | 13 | Target | ✅ Implemented | `make EXAMPLE=13-06-event-driven-demo flash` |
| `14-memory-allocator-lab` | 14 | Host + Target | ✅ Implemented | `make phase14-lab`; target: `make EXAMPLE=14-memory-allocator-lab flash` |
| `15-kernel-benchmark` | 15 | Target | ✅ Implemented | `make EXAMPLE=15-kernel-benchmark flash` |
| `16-diagnostics-stress-stabilization` | 16 | Host + Target | ✅ Implemented | `make phase16-stress`; target: `make EXAMPLE=16-diagnostics-stress-stabilization flash` |

## Host commands

Run the standalone host examples/labs with:

```bash
make phase2-example
make phase14-lab
make phase16-stress
```

Run all completed host-side unit tests, including HairEvent, allocator, diagnostics, and stress
validation tests:

```bash
make host-tests
```

Never run:

```bash
make EXAMPLE=02-kernel-data-structures-host flash
```

The Makefile rejects that command because the host example is not freestanding
STM32 firmware.

## Phase 14 allocator sequence

```bash
make phase14-lab
make EXAMPLE=14-memory-allocator-lab flash
make phase14-check
```

The native lab prints heap/pool statistics. The target example uses separate
static arenas and reports first-fit, coalescing, and fragmentation results over
USART1. The allocator is not used by the kernel.

## Phase 15 benchmark sequence

```bash
make EXAMPLE=15-kernel-benchmark
make EXAMPLE=15-kernel-benchmark flash
make phase15-check
```

The Phase 15 image runs only on the STM32 target because SVC, PendSV, PSP,
DWT_CYCCNT, and exception preemption cannot be represented by a native host
program. Generic statistics logic remains covered by `make host-tests`.


## Phase 16 stabilization sequence

```bash
make phase16-stress
make EXAMPLE=16-diagnostics-stress-stabilization flash
make phase16-check
```

The host command runs a reproducible 500,000-operation scheduler stress. The
target continuously exercises queue, semaphore, mutex, software timer, timeout,
preemption, stack guards, retained fault records, and health monitoring.

## Current runnable set

```text
Host:
  02-kernel-data-structures-host
  Phase 14 native allocator lab (`make phase14-lab`)
  Phase 16 deterministic scheduler stress (`make phase16-stress`)

Target:
  01-baremetal-foundation
  03-static-task-stack
  04-start-first-task
  05-cooperative-context-switch
  06-priority-scheduler
  07-task-delay-timeout
  08-preemption-round-robin
  09-queue-blocking-ipc
  10-01-semaphore-from-isr
  10-02-mutex-priority-inheritance
  11-task-suspend-resume
  12-software-timer
  13-01-event-post
  13-02-active-object
  13-03-flat-state-machine
  13-04-time-event
  13-05-publish-subscribe
  13-06-event-driven-demo
  14-memory-allocator-lab
  15-kernel-benchmark
  16-diagnostics-stress-stabilization
```
