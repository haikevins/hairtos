# hairtos examples

Numbered examples follow the development roadmap. They remain educational snapshots. The unnumbered `hairtos` example is the current integrated product image.

## Unified command layout

Every selected example uses the same actions:

```bash
make EXAMPLE=<name> build
make EXAMPLE=<name> run
make EXAMPLE=<name> check
make EXAMPLE=<name> clean
```

- For a **host** example, `run` executes the native binary.
- For a **target** example, `run` builds, flashes, verifies, and resets the STM32.
- `ENVIRONMENT=auto` is the default.
- `14-memory-allocator-lab` and `hairtos` support both environments, so specify `ENVIRONMENT=host` for their native variants.

## Example matrix

| Folder | Phase | Environment | Purpose | Run command |
|---|---:|---|---|---|
| `01-baremetal-foundation` | 1 | Target | Bare-metal clock, GPIO, UART, and temporary SysTick | `make EXAMPLE=01-baremetal-foundation run` |
| `02-kernel-data-structures-host` | 2 | Host | Intrusive lists, ready set, and wait-list ordering | `make EXAMPLE=02-kernel-data-structures-host run` |
| `03-static-task-stack` | 3 | Target | TCB and Cortex-M3 initial task stack | `make EXAMPLE=03-static-task-stack run` |
| `04-start-first-task` | 4 | Target | Start the first task through SVC | `make EXAMPLE=04-start-first-task run` |
| `05-cooperative-context-switch` | 5 | Target | PendSV cooperative switching | `make EXAMPLE=05-cooperative-context-switch run` |
| `06-priority-scheduler` | 6 | Target | Fixed-priority scheduler and equal-priority FIFO | `make EXAMPLE=06-priority-scheduler run` |
| `07-task-delay-timeout` | 7 | Target | Kernel SysTick, delay, and timeout wake-up | `make EXAMPLE=07-task-delay-timeout run` |
| `08-preemption-round-robin` | 8 | Target | Preemption and round-robin | `make EXAMPLE=08-preemption-round-robin run` |
| `09-queue-blocking-ipc` | 9 | Target | Blocking queue and timeout | `make EXAMPLE=09-queue-blocking-ipc run` |
| `10-01-semaphore-from-isr` | 10 | Target | Semaphore give from ISR | `make EXAMPLE=10-01-semaphore-from-isr run` |
| `10-02-mutex-priority-inheritance` | 10 | Target | Mutex ownership and priority inheritance | `make EXAMPLE=10-02-mutex-priority-inheritance run` |
| `11-task-suspend-resume` | 11 | Target | Suspend and resume | `make EXAMPLE=11-task-suspend-resume run` |
| `12-software-timer` | 12 | Target | Software timer service | `make EXAMPLE=12-software-timer run` |
| `13-01-event-post` | 13 | Target | Post HairEvent from ISR | `make EXAMPLE=13-01-event-post run` |
| `13-02-active-object` | 13 | Target | Active Object dispatch | `make EXAMPLE=13-02-active-object run` |
| `13-03-flat-state-machine` | 13 | Target | Flat state machine | `make EXAMPLE=13-03-flat-state-machine run` |
| `13-04-time-event` | 13 | Target | HairEvent time event | `make EXAMPLE=13-04-time-event run` |
| `13-05-publish-subscribe` | 13 | Target | Publish/subscribe ownership | `make EXAMPLE=13-05-publish-subscribe run` |
| `13-06-event-driven-demo` | 13 | Target | Integrated HairEvent demo | `make EXAMPLE=13-06-event-driven-demo run` |
| `14-memory-allocator-lab` | 14 | Host + Target | Fixed-block pool and first-fit heap lab | Host: `make ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run`<br>Target: `make ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab run` |
| `15-kernel-benchmark` | 15 | Target | DWT/GPIO kernel benchmark | `make EXAMPLE=15-kernel-benchmark run` |
| `hairtos` | Current | Host + Target | Integrated kernel diagnostics and deterministic scheduler stress | Host: `make ENVIRONMENT=host EXAMPLE=hairtos run`<br>Target: `make ENVIRONMENT=target EXAMPLE=hairtos run` |

## Common commands

```bash
make help
make list-examples
make host-tests
make clean-all
```

Examples:

```bash
make EXAMPLE=02-kernel-data-structures-host run
make EXAMPLE=08-preemption-round-robin build
make EXAMPLE=08-preemption-round-robin run
make ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run
make ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab run
make EXAMPLE=hairtos run
```

`check` runs the complete host sanitizer suite and builds the selected host or target example. A successful target build does not prove runtime behavior on physical hardware.
