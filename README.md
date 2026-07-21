# HairRTOS

HairRTOS is an educational fixed-priority preemptive RTOS for ARM Cortex-M.
The first target is the STM32F103C8T6 Blue Pill. HairEvent is an optional
Event-Driven framework that will be built above the kernel.

## Current status: Phase 1 complete

The repository now contains a runnable register-level bare-metal foundation:

- STM32F103 startup and complete medium-density vector table;
- linker script for 64 KiB flash and 20 KiB RAM;
- HSE/PLL 72 MHz clock setup with HSI fallback;
- GPIO PC13 onboard LED support;
- USART1 polling output on PA9 at 115200 baud;
- 1 kHz SysTick millisecond time base;
- Makefile/CMake, OpenOCD, and GDB setup;
- `examples/00-baremetal-blink`.

The RTOS scheduler is intentionally not implemented yet.

## Build

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

Run checks:

```bash
make phase0-check
make phase1-check
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

Read `docs/phase1-baremetal-foundation.md` for the Phase 1 boot flow and test
criteria. The next phase implements intrusive lists and scheduler data structures
with host-side tests.
