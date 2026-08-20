# Example index

> The 22 example directories form a deliberate progression from bare metal to diagnostics/stress testing.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [Next →](memory-allocator-lab.md)

| Example | Focus | Environment |
| --- | --- | --- |
| [`01-baremetal-foundation`](../../examples/01-baremetal-foundation/README.md) | `01-baremetal-foundation` — Bare-Metal Foundation | Target |
| [`02-kernel-data-structures-host`](../../examples/02-kernel-data-structures-host/README.md) | `02-kernel-data-structures-host` — Kernel Data Structures — Host Demo | Host only |
| [`03-static-task-stack`](../../examples/03-static-task-stack/README.md) | `03-static-task-stack` — Static TCB and Initial Task Stack | Target |
| [`04-start-first-task`](../../examples/04-start-first-task/README.md) | `04-start-first-task` — Starting the First Task with SVC | Target |
| [`05-cooperative-context-switch`](../../examples/05-cooperative-context-switch/README.md) | `05-cooperative-context-switch` — Cooperative Context Switching | Target |
| [`06-priority-scheduler`](../../examples/06-priority-scheduler/README.md) | `06-priority-scheduler` — Fixed-Priority Scheduler | Target |
| [`07-task-delay-timeout`](../../examples/07-task-delay-timeout/README.md) | `07-task-delay-timeout` — SysTick, Task Delay, and Timeouts | Target |
| [`08-preemption-round-robin`](../../examples/08-preemption-round-robin/README.md) | `08-preemption-round-robin` — Preemption and Round-Robin | Target |
| [`09-queue-blocking-ipc`](../../examples/09-queue-blocking-ipc/README.md) | `09-queue-blocking-ipc` — Queue and Blocking IPC | Target |
| [`10-01-semaphore-from-isr`](../../examples/10-01-semaphore-from-isr/README.md) | `10-01-semaphore-from-isr` — Giving a Semaphore from an ISR | Target |
| [`10-02-mutex-priority-inheritance`](../../examples/10-02-mutex-priority-inheritance/README.md) | `10-02-mutex-priority-inheritance` — Mutex and Priority Inheritance | Target |
| [`11-task-suspend-resume`](../../examples/11-task-suspend-resume/README.md) | `11-task-suspend-resume` — Task Suspend and Resume | Target |
| [`12-software-timer`](../../examples/12-software-timer/README.md) | `12-software-timer` — Software Timer Service | Target |
| [`13-01-event-post`](../../examples/13-01-event-post/README.md) | `13-01-event-post` — Posting haievent Events from an ISR | Target |
| [`13-02-active-object`](../../examples/13-02-active-object/README.md) | `13-02-active-object` — Active Object Ping–Pong | Target |
| [`13-03-flat-state-machine`](../../examples/13-03-flat-state-machine/README.md) | `13-03-flat-state-machine` — Flat State Machine | Target |
| [`13-04-time-event`](../../examples/13-04-time-event/README.md) | `13-04-time-event` — haievent Time Events | Target |
| [`13-05-publish-subscribe`](../../examples/13-05-publish-subscribe/README.md) | `13-05-publish-subscribe` — Publish–Subscribe and Dynamic-Event Ownership | Target |
| [`13-06-event-driven-demo`](../../examples/13-06-event-driven-demo/README.md) | `13-06-event-driven-demo` — Integrated haievent Demo | Target |
| [`14-memory-allocator-lab`](../../examples/14-memory-allocator-lab/README.md) | `14-memory-allocator-lab` — Memory Allocator Lab | Host + target |
| [`15-kernel-benchmark`](../../examples/15-kernel-benchmark/README.md) | `15-kernel-benchmark` — Benchmark kernel | Target |
| [`16-diagnostics-stress-stabilization`](../../examples/16-diagnostics-stress-stabilization/README.md) | `16-diagnostics-stress-stabilization` — Diagnostics and Stress-Test Stabilization | Host + target |

## Dependency progression

Examples 03–06 build stack setup, first-task startup, context switching, and scheduling before example 07 introduces kernel-tick-based blocking. Examples 09–12 add synchronization one primitive at a time. The 13-* series appears only after queue/timer/context support is mature enough for Active Objects. Example 14 isolates the allocator from the kernel; 15 measures performance; 16 integrates stress and diagnostics.

## CMake truth

Each example's environment, module set, and feature definitions are declared in `cmake/hairtos_examples.cmake`, which is the source of truth for build composition.

## Validation

Host validation baseline: examples 02/14/16 PASS. Target examples require the ARM toolchain and a Blue Pill board.
