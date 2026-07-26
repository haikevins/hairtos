# Changelog

## 1.0.0-rc1 — Phase 16 diagnostics and stabilization

- Added retained `.noinit` panic/fault records and strong Cortex-M3 fault handlers.
- Added runtime counters, task snapshots, stack checks, and whole-kernel invariant validation.
- Added `HR_ASSERT`, weak panic/stack/assert hooks, and optional UsageFault injection.
- Added a 500,000-operation native scheduler stress and mixed-IPC target stress image.
- Added API, porting, Cortex-M0 compile-proof, stress-plan, and release-checklist docs.
- Removed the obsolete duplicate `framework/event` placeholder tree.
- Marked the Phase 0–16 source roadmap complete.

## Unreleased

### Added — Phase 15: Kernel benchmark

- Added Cortex-M3 DWT_CYCCNT cycle measurements and PB0 GPIO timing markers.
- Added fixed-capacity statistics with min, max, mean, p50, p95, overhead subtraction, and wrap-safe elapsed cycles.
- Added SVC startup, critical-section, scheduler, queue, semaphore, mutex, timer-command, PendSV round-trip, queue wake/preempt, HairEvent dispatch, and timer-jitter metrics.
- Deferred USART1 reporting until collection completes and reported build conditions, Flash/RAM use, and stack high-water marks.
- Added 59/59 host sanitizer coverage, Make/CMake integration, target symbol/disassembly checks, and Phase 15 documentation.
- Kept benchmark sources out of normal HairRTOS target builds.

### Added — Phase 14: Memory allocator lab

- Added an isolated fixed-block pool with aligned caller-provided storage.
- Added a first-fit heap with aligned payloads, splitting, and bidirectional coalescing.
- Added invalid-pointer, middle-pointer, double-free, exhaustion, and structural checks.
- Added internal/external fragmentation and allocator usage statistics.
- Added deterministic randomized ASan/UBSan tests, a native demo, and an STM32 UART example.
- Kept HairRTOS and HairEvent static-first with no allocator dependency.

### Added — Phase 13: HairEvent framework

- Added immutable static events and fixed-block dynamic event pools.
- Added reference-counted direct-post and multicast ownership semantics.
- Added statically allocated Active Objects backed by HairRTOS tasks and queues.
- Added flat run-to-completion state machines with ENTRY, EXIT, and INIT signals.
- Added software-timer-backed time events that post into Active Object queues.
- Added fixed-capacity publish/subscribe with caller-provided subscriber storage.
- Added ISR event-post, Active Object, flat-SM, time-event, pub/sub, and integration examples.
- Added HairEvent host tests, documentation, Make/CMake integration, and Phase 13 validation.

### Added — Phase 12: Software timer service

- Added opaque static one-shot and auto-reload software timers.
- Added ordered timer deadlines, pending-expiration accounting, and FIFO ordering.
- Added lazy static timer-service task initialization and semaphore wake-up.
- Added start, stop, reset, and period-change task-context APIs.
- Kept callbacks outside SysTick and outside kernel critical sections.
- Added Phase 12 host tests, STM32 example, documentation, and validation.

### Added — Phase 11: Task suspend and resume

- Added `hr_task_suspend()` and `hr_task_resume()`.
- Added READY, RUNNING/self-suspend, and BLOCKED suspension policies.
- Preserved wait-list and timeout state while administratively suspended.
- Added deferred readiness and resume-triggered higher-priority preemption.
- Added Phase 11 target example, documentation, build integration, and validation.


### Added — Phase 10: Semaphore, mutex, and priority inheritance

- Added opaque binary/counting semaphores with blocking take and ISR-safe give.
- Added mutex ownership validation, direct waiter ownership handoff, and timeout.
- Added base/effective priority tracking across all mutexes held by a task.
- Added chained priority inheritance and timeout-driven inheritance restoration.
- Added EXTI software-interrupt semaphore and priority-inversion target examples.
- Added Phase 10 host tests, documentation, build integration, and validation.

### Added — Phase 9: Queue and blocking IPC

- Added opaque statically allocated fixed-size FIFO queues.
- Added blocking and non-blocking send/receive with finite timeout and wait forever.
- Added priority/FIFO sender and receiver wait lists with direct handoff.
- Added ISR-safe non-blocking send and receive variants.
- Added timeout cleanup across object and global timeout lists.
- Added Phase 9 host tests, STM32 example, documentation, and validation.

### Added — Phase 8: Preemption and round-robin

- Added strict higher-priority preemption after timeout wake-up.
- Added tick-driven equal-priority round-robin with per-task quantum.
- Added deferred switch-reason tracking for yield, block, preemption, and time slice.
- Added Phase 8 host coverage, STM32 target example, documentation, and validation.
- Made PendSV scheduler selection atomic with interrupt masking.

### Added — Phase 7: SysTick and delay

- Added kernel-owned 1 kHz SysTick and monotonic tick time.
- Added blocking relative and periodic task delays.
- Added timeout wake-up, tick-wrap handling, and idle-to-task PendSV switching.
- Split the legacy bare-metal SysTick IRQ from the timer driver.
- Added PRIMASK critical-section primitives and ISR-context detection.
- Added Phase 7 host coverage, target example, documentation, and validation.

### Added — Phase 6: Fixed-priority scheduler

- Added an explicit scheduler policy layer above the Phase 2 ready set.
- Added highest-priority selection independent of task registration order.
- Added FIFO cooperative rotation among equal-priority tasks.
- Added rejection of yield attempts from a non-selected lower-priority task.
- Added mixed-priority host tests and the Phase 6 STM32 target example.
- Added Phase 6 documentation, target build, symbol, and disassembly validation.


### Added — Phase 5: Cooperative context switch

- Added `hr_task_yield()` and Cortex-M PendSV request support.
- Added PendSV R4-R11/PSP save and restore in GNU Assembly.
- Added cooperative ready-queue rotation and current-task handoff.
- Added two-task target example with independent local-state checks.
- Added Phase 5 host tests, documentation, target build, and disassembly validation.

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
