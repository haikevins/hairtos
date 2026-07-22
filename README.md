# HairRTOS

HairRTOS is an educational fixed-priority preemptive RTOS for ARM Cortex-M.
The first target is the STM32F103C8T6 Blue Pill. HairEvent is an optional
Event-Driven framework built above the kernel.

## Current status: Phase 1 complete

The repository currently contains only the runnable register-level bare-metal
foundation:

- STM32F103 startup and complete medium-density vector table;
- linker script for 64 KiB Flash and 20 KiB RAM;
- HSE/PLL 72 MHz clock setup with HSI fallback;
- GPIO PC13 onboard LED support;
- USART1 polling output on PA9 at 115200 baud;
- 1 kHz SysTick millisecond time base;
- Makefile/CMake, OpenOCD, and GDB setup;
- `examples/00-baremetal-blink`.

The RTOS scheduler, task switching, synchronization, HairEvent, allocator lab,
and benchmark suite are **not implemented yet**. Their documentation and
placeholder directories exist only to define the work that follows Phase 1.

## Build Phase 1

Install the GNU Arm Embedded toolchain, then run:

```bash
make
```

Local validation with Clang/LLD is also supported:

```bash
make TOOLCHAIN=clang
```

Flash through ST-Link V2:

```bash
make flash
```

Run specification and structure checks:

```bash
make phase0-check
make phase1-check
make roadmap-check
```

## UART

USART1 uses PA9 TX and PA10 RX at `115200 8-N-1`.

## Architecture

```text
Application
    -> HairEvent framework
    -> HairRTOS kernel
    -> Cortex-M port
    -> STM32F1 SoC
    -> Blue Pill board and drivers
```

## Official roadmap

| Phase | Scope | Current state |
|---:|---|---|
| 0 | Specification and principles | Complete |
| 1 | Bare-metal foundation | **Complete** |
| 2 | Intrusive lists and kernel data structures | Not started |
| 3 | TCB and initial task stack | Not started |
| 4 | Start first task using SVC | Not started |
| 5 | PendSV cooperative context switch | Not started |
| 6 | Fixed-priority scheduler | Not started |
| 7 | SysTick, delay, and timeout base | Not started |
| 8 | Preemption and round-robin | Not started |
| 9 | Queue and blocking IPC | Not started |
| 10 | Semaphore, mutex, and priority inheritance | Not started |
| 11 | Task suspend/resume | Not started |
| 12 | Software timer service | Not started |
| 13 | HairEvent framework | Not started |
| 14 | Memory allocator lab | Not started |
| 15 | Kernel benchmarks | Not started |
| 16 | Diagnostics, stress, and stabilization | Not started |

The allocator remains an educational lab and is not required by the static-first
v1.0 kernel. Benchmark work is performed only after kernel behavior is correct.

Read:

- `docs/phase1-baremetal-foundation.md`
- `docs/roadmap.md`
- `docs/requirements-coverage.md`
- `docs/task-suspend-resume.md`
- `docs/memory-allocator-lab.md`
- `docs/benchmark-plan.md`

The next implementation phase is **Phase 2 — intrusive lists and kernel data
structures with host-side tests**.
