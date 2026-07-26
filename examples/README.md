# HairRTOS Examples

Example directory numbers follow the official HairRTOS roadmap phase numbers.
Phase 0 has no runtime example because it contains specifications only.

## Environment legend

| Environment | Meaning |
|---|---|
| **Host** | Native Ubuntu executable built with the system `cc`, GCC, or Clang. It does not use STM32 startup code and cannot be flashed. |
| **Target** | Cortex-M3 firmware cross-compiled for the STM32F103C8T6 Blue Pill. It may be flashed only after that phase is implemented. |
| **Host + Target** | The phase is planned to include both native tests/labs and an STM32 demonstration. |

## Example matrix

| Folder | Phase | Environment | Status | How to run now |
|---|---:|---|---|---|
| `01-baremetal-foundation` | 1 | Target | ✅ Implemented | `make EXAMPLE=01-baremetal-foundation flash` |
| `02-kernel-data-structures-host` | 2 | Host | ✅ Implemented | `make phase2-example` |
| `03-static-task-stack` | 3 | Target | ✅ Implemented | `make EXAMPLE=03-static-task-stack flash` |
| `04-start-first-task` | 4 | Target | ✅ Implemented | `make EXAMPLE=04-start-first-task flash` |
| `05-cooperative-context-switch` | 5 | Target | ⬜ Placeholder | Not runnable yet |
| `06-priority-scheduler` | 6 | Target | ⬜ Placeholder | Not runnable yet |
| `07-task-delay-timeout` | 7 | Target | ⬜ Placeholder | Not runnable yet |
| `08-preemption-round-robin` | 8 | Target | ⬜ Placeholder | Not runnable yet |
| `09-queue-blocking-ipc` | 9 | Target | ⬜ Placeholder | Not runnable yet |
| `10-01-semaphore-from-isr` | 10 | Target | ⬜ Placeholder | Not runnable yet |
| `10-02-mutex-priority-inheritance` | 10 | Target | ⬜ Placeholder | Not runnable yet |
| `11-task-suspend-resume` | 11 | Target | ⬜ Placeholder | Not runnable yet |
| `12-software-timer` | 12 | Target | ⬜ Placeholder | Not runnable yet |
| `13-01-event-post` | 13 | Target | ⬜ Placeholder | Not runnable yet |
| `13-02-active-object` | 13 | Target | ⬜ Placeholder | Not runnable yet |
| `13-03-flat-state-machine` | 13 | Target | ⬜ Placeholder | Not runnable yet |
| `13-04-time-event` | 13 | Target | ⬜ Placeholder | Not runnable yet |
| `13-05-publish-subscribe` | 13 | Target | ⬜ Placeholder | Not runnable yet |
| `13-06-event-driven-demo` | 13 | Target | ⬜ Placeholder | Not runnable yet |
| `14-memory-allocator-lab` | 14 | Host + Target | ⬜ Placeholder | Not runnable yet |
| `15-kernel-benchmark` | 15 | Target | ⬜ Placeholder | Not runnable yet |
| `16-diagnostics-stress-stabilization` | 16 | Host + Target | ⬜ Placeholder | Not runnable yet |

## Important command rules

### Host example

Run the only implemented host example with:

```bash
make phase2-example
```

Run all completed host-side unit tests with:

```bash
make host-tests
```

Never run:

```bash
make EXAMPLE=02-kernel-data-structures-host flash
```

That folder uses native facilities such as `printf()` and is not linked as
freestanding STM32 firmware.

### Target examples

Always pass the target example name in the same `make` invocation used for
flashing:

```bash
make EXAMPLE=01-baremetal-foundation flash
make EXAMPLE=03-static-task-stack flash
make EXAMPLE=04-start-first-task flash
```

A previous command such as `make EXAMPLE=03-static-task-stack` does not make the
next standalone `make flash` remember that selection; without `EXAMPLE`, Make
uses its default target example.

## Current implemented examples

At Phase 4, the runnable set is exactly:

```text
Host:
  02-kernel-data-structures-host

Target:
  01-baremetal-foundation
  03-static-task-stack
  04-start-first-task
```

All other example directories are roadmap placeholders.
