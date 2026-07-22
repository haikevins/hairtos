# Changelog

## Unreleased

### Updated — Roadmap refinement while remaining at Phase 1

- Added the official Phase 0–16 implementation roadmap.
- Added explicit Task Suspend/Resume planning.
- Added an isolated first-fit Memory Allocator Lab.
- Added a Kernel Benchmark plan using DWT, GPIO, and deferred UART reporting.
- Added requirements-coverage documentation and future placeholder directories.
- Kept the current implementation status at Phase 1; no scheduler or later-phase
  kernel feature was implemented.

### Added — Phase 1

- Buildable STM32F103C8T6 bare-metal target.
- Reset handler, vector table, `.data` initialization, and `.bss` clearing.
- Blue Pill linker script for 64 KiB flash and 20 KiB RAM.
- 72 MHz HSE/PLL clock configuration with HSI fallback.
- Register-level GPIO, USART1, and SysTick drivers.
- PC13 LED and UART board support.
- Bare-metal blink/heartbeat example.
- GNU Arm and Clang/LLD build paths.
- ST-Link/OpenOCD and GDB configuration without an NRST requirement.
- Phase 1 validation script and documentation.

### Added — Phase 0

- Initial directory structure and architecture specifications.
