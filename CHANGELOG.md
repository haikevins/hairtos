# Changelog

## Unreleased

### Added — Phase 4: Start first task using SVC

- Added kernel lifecycle state and static idle task initialization.
- Added `hr_task_start()` and first READY/RUNNING transitions.
- Added first-task selection from the priority ready set.
- Added Cortex-M3 SVC assembly startup and PSP exception return.
- Added Phase 4 host tests, target example, documentation, and validation.
- Added Makefile rejection for attempts to flash host-only examples.

### Documentation

- Added the complete Phase 0-16 roadmap status table to the root README.
- Added `examples/README.md` with explicit Host, Target, and Host + Target classification.
- Documented the correct build/flash commands and the Phase 2 host-only restriction.


### Fixed — Roadmap-aligned example layout

- Renamed all example directories to match official Phase 1–16 numbering.
- Split the two Phase 10 examples and six Phase 13 examples with sub-numbers.
- Added the Phase 2 host demonstration and Phase 16 placeholder.
- Removed stale Phase 0–2 manifest snapshots that referenced legacy paths.
- Added `make example-layout-check` to prevent duplicate or legacy numbering.


### Added — Phase 3 TCB and initial task stack

- Added opaque aligned static task storage and an internal TCB.
- Added Cortex-M3 raw initial stack-frame construction.
- Added task stack fill, guard, and high-watermark diagnostics.
- Added host tests and the `03-static-task-stack` target example.
- Added `make phase3-check`.


### Added — Phase 2 kernel data structures

- Circular intrusive doubly linked list with membership validation.
- Priority ready set with FIFO queues and bitmap.
- Priority-ordered wait list with equal-priority FIFO.
- Dual timeout lists with 32-bit tick-wrap handling.
- Host tests with AddressSanitizer and UndefinedBehaviorSanitizer.
- Phase 2 validation script and documentation.

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
