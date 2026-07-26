# hairtos

`hairtos` is a static-first educational real-time operating system for ARM Cortex-M.
The current reference target is the STM32F103C8T6 Blue Pill. `haievent` is the
optional event-driven framework implemented above the kernel.

## Current status

The repository is the **hairtos mainline**, while the integrated diagnostics image keeps the historical numbered example `16-diagnostics-stress-stabilization`. It
contains the complete implementation developed through the roadmap milestones:

- static task creation and Cortex-M3 initial stack frames;
- SVC first-task startup and PendSV context switching;
- fixed-priority preemptive scheduling and equal-priority round-robin;
- SysTick, delay, timeout, queue, semaphore, mutex and priority inheritance;
- suspend/resume and task-context software timers;
- haievent Active Objects, flat state machines, time events and publish/subscribe;
- allocator laboratory, DWT benchmark, retained fault diagnostics and stress tests.

The numbered examples remain as a learning path. The fully integrated image is
selected with `EXAMPLE=16-diagnostics-stress-stabilization`.

## Unified Make commands

Host and target examples use the same layout:

```bash
make EXAMPLE=<name> build [ENVIRONMENT=host|target]
make EXAMPLE=<name> run   [ENVIRONMENT=host|target]
make EXAMPLE=<name> check [ENVIRONMENT=host|target]
make EXAMPLE=<name> clean [ENVIRONMENT=host|target]
```

The default command builds the integrated target image:

```bash
make build
# equivalent to:
make EXAMPLE=16-diagnostics-stress-stabilization ENVIRONMENT=target build
```

Common examples:

```bash
# Integrated STM32 image: build, flash, verify and reset
make EXAMPLE=16-diagnostics-stress-stabilization run

# Native diagnostics/scheduler stress
make ENVIRONMENT=host EXAMPLE=16-diagnostics-stress-stabilization run

# Host-only kernel data-structure example
make EXAMPLE=02-kernel-data-structures-host run

# Full sanitizer suite plus selected build
make TOOLCHAIN=clang EXAMPLE=16-diagnostics-stress-stabilization check
```

Run `make help` for the compact command summary and `make list-examples` for the
complete host/target classification.

## Validation boundary

`make check` runs the native sanitizer suite and builds the selected example.
A successful cross-build does not replace runtime validation on physical STM32
hardware. Retained fault records, long-duration stress and timing measurements
must still be verified on the board.

## Documentation

Start with:

- `docs/README.md`
- `docs/00-overview/architecture.md`
- `docs/00-overview/roadmap.md`
- `docs/05-api-reference/README.md`
- `docs/06-testing-and-quality/testing-guide.md`
- `examples/README.md`

Repository cleanup rules are documented in
`docs/06-testing-and-quality/repository-hygiene.md`.
