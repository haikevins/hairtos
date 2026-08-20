# hairtos

> **Source version:** `1.0.0-rc1`  
> **Focus:** static-first preemptive RTOS kernel + event-driven framework (`haievent`) + flat state machine / Active Object  
> **Reference target:** `bluepill_f103c8` — STM32F103C8T6 / ARM Cortex-M3

`hairtos` is an educational and experimental RTOS project whose implementation is organized like a real systems codebase: clear public/internal boundaries, static object ownership, explicit scheduler/timeout/IPC invariants, a Cortex-M3 context switch implemented in the architecture port, host sanitizer tests, a target manifest, and an independent event-driven framework running on the kernel itself.

The most important distinction when reading this repository is **roadmap versus implementation**. `docs/00`–`08`, source, and examples describe the current v1 implementation. `docs/09-version2/` contains future design proposals only.

## Table of Contents

- [System Architecture](#architecture)
- [Kernel v1](#kernel)
- [`haievent`](#haievent)
- [Cortex-M3 / STM32F103 target](#target)
- [Build, Test, and Evidence](#build)
- [Example Roadmap](#examples)
- [Repository map](#repo-map)
- [Documentation](#docs)
- [v1 Limitations](#limits)
- [References](#references)

<a id="architecture"></a>
## System Architecture

**Runtime dependency path**

```mermaid
flowchart TB
    APP["Application / examples"] --> HE["haievent API"]
    APP --> HR["hairtos API"]
    HE --> K["Kernel / framework core"]
    HR --> K
```

**Target binding path**

```mermaid
flowchart TB
    K["Kernel core"] --> PORT["Port contract"]
    PORT --> CM3["Cortex-M3 port"]
    K --> BOARD["Board services"]
    BOARD --> DRV["Driver contracts"]
    DRV --> SOC["STM32F1 backend"]
    MAN["Target manifest"] -.-> CM3
    MAN -.-> SOC
    MAN -.-> BOARD
```

### Primary Boundaries

- **Application** code should include only `hairtos/hairtos.h`, `haievent/haievent.h`, and `board.h`.
- **Public kernel objects** (`hr_task_t`, `hr_queue_t`, `hr_mutex_t`, ...) are opaque fixed-size storage. Internal control blocks reside inside union storage, and `_Static_assert` verifies that the configured storage is large enough.
- **Kernel internals** own ready/wait/timeout lists, scheduler policy, blocking/wakeup protocols, and object-specific invariants.
- **Architecture port** owns PSP/MSP handling, SVC, PendSV, critical sections, and context save/restore.
- **SoC/board/driver** layers isolate registers, clocks, pins, and peripherals from the generic kernel.

<a id="kernel"></a>
## Kernel v1

### Scheduling and Tasks

- Fixed-priority preemptive scheduling; **lower numeric values represent higher priority**.
- `8 priority levels; lower numbers mean higher priority; idle runs at priority 7`.
- One intrusive FIFO ready queue per priority plus a bitmap for non-empty priority levels.
- Equal-priority round-robin when `HR_CFG_TIME_SLICING=1`; default slice = 1 tick.
- Tasks are created from caller-owned `hr_task_t` objects plus stacks; there is no dynamic kernel allocation.
- Task model: CREATED, READY, RUNNING, BLOCKED, SUSPENDED.
- Base and effective priority are stored separately to support priority inheritance.

### Time, Blocking, and IPC

- Tick type is `uint32_t`; default frequency is `1 kHz`.
- Timeouts use two sorted lists (`current`/`overflow`) to handle tick wrap-around.
- FIFO queues provide blocking send/receive, timeouts, non-blocking ISR APIs, and direct handoff.
- Counting/binary semaphores; ISR give can wake a task.
- Mutex normal/recursive; ownership + chained priority inheritance + direct handoff.
- Software-timer callbacks execute in the timer-service task, not directly in the SysTick ISR.

### Diagnostics

- Stack fill `0xA5`, guard `0xDEADBEEF`, high-watermark.
- Runtime counters and health reports.
- Panic/fault records are stored in `.noinit` so they can be inspected after reset.
- Fault context stores stacked registers plus CFSR/HFSR/DFSR/AFSR/MMFAR/BFAR/SHCSR.

<a id="haievent"></a>
## `haievent`

```mermaid
flowchart TB
    PRODUCER["Producer"] --> POST["Post event"]
    POST --> AOQ["AO queue"]
    AOQ --> AOT["AO task"]
    AOT --> FSM["RTC dispatch"]
    FSM --> OWN["Release dynamic event"]
```

v1 provides:

- static events and dynamic events from a fixed-block pool;
- reference counting;
- a flat state machine with reserved ENTRY/EXIT/INIT/TIMEOUT signals;
- Active Object = task + queue + state machine;
- time events built on software timers;
- publish/subscribe with a static subscriber table;
- posting from task and ISR context.

v1 **does not yet** provide HSMs, deferred events, history states, or a shared-executor AO model.

<a id="target"></a>
## Cortex-M3 / STM32F103 target

Current complete target: `bluepill_f103c8`.

| Attribute | Current Binding |
| --- | --- |
| MCU | STM32F103C8T6 |
| CPU | ARM Cortex-M3 |
| Clock | Nominal 72 MHz (HSE 8 MHz → PLL ×9), with HSI fallback |
| UART | USART1 PA9/PA10, 115200 8-N-1 |
| LED | PC13 active-low |
| Kernel tick | SysTick, 1 kHz |
| Context start/switch | SVC / PendSV |
| Benchmark clock | DWT CYCCNT |
| Benchmark marker | PB0 active-high |
| Debug | ST-Link + SWD + OpenOCD/GDB |

Cortex-M3 context path:

**First-task start**

```mermaid
sequenceDiagram
    participant M as main / MSP
    participant S as SVC
    participant T as first task / PSP
    M->>S: start kernel via SVC
    S->>T: restore software context
    S-->>T: exception return on PSP
```

**Subsequent context switch**

```mermaid
sequenceDiagram
    participant T as current task
    participant P as PendSV
    participant K as kernel selector
    T->>P: switch requested
    P->>P: save R4-R11
    P->>K: select next task
    K-->>P: update current TCB
    P->>P: restore next R4-R11
    P-->>T: exception return
```

<a id="build"></a>
## Build, Test, and Evidence

Makefile is a command wrapper; CMake is the source of truth for target/example/module/source selection.

```bash
make help
make list-targets
make list-examples
```

Target build:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=16-diagnostics-stress-stabilization build
```

Host tests:

```bash
make TARGET=bluepill_f103c8 host-tests
```

### Validation baseline

- GCC host build + AddressSanitizer + UndefinedBehaviorSanitizer: **PASS**.
- `ctest`: **PASS**.
- 64 host test functions are built into the suite.
- `02-kernel-data-structures-host`: **PASS**.
- `14-memory-allocator-lab` host demo: **PASS**.
- `16-diagnostics-stress-stabilization` host stress: **PASS**, 500,000 iterations.

Host validation does not prove that target firmware cross-builds or runs on hardware; that requires the ARM GNU toolchain, OpenOCD, and a `bluepill_f103c8` board.

<a id="examples"></a>
## Example Roadmap

| Stage | Examples | Mechanism |
| --- | --- | --- |
| Bare-metal | 01 | board/UART/LED/tick baseline |
| Kernel structures | 02 | intrusive list, ready set, wait list |
| Task bootstrap | 03–05 | TCB, stack frame, SVC, PendSV |
| Scheduling/time | 06–08 | priority, blocking delay, preemption, round-robin |
| IPC/sync | 09–12 | queue, semaphore, mutex PI, suspend/resume, timer |
| Event-driven | 13-01…13-06 | event, AO, FSM, time event, pub-sub, integration |
| Experiments/evidence | 14–16 | allocator, benchmark, diagnostics/stress |

Details: [`examples/README.md`](examples/README.md).

<a id="repo-map"></a>
## Repository map

```text
hairtos/
├── arch/                     # architecture-specific context/critical/fault/benchmark
├── benchmarks/kernel/        # generic benchmark statistics
├── boards/                   # board binding, linker, marker/UART/LED services
├── cmake/                    # target/module/example source-of-truth
├── config/                   # compile-time kernel + haievent policy
├── docs/                     # technical docs v1 + explicit v2 roadmap
├── drivers/                  # public peripheral contracts + STM32F1 backend
├── examples/                 # learning/evidence sequence 01–16
├── haievent/                 # event framework
├── kernel/                   # public/internal/source of RTOS kernel
├── labs/memory-allocator/    # allocator experiment outside kernel runtime
├── soc/                      # STM32F1 startup/clock/IRQ/register layer
├── tests/                    # host, mocks, portability probes, stress
└── tools/                    # OpenOCD/GDB helpers
```

<a id="docs"></a>
## Documentation

Start at [`docs/README.md`](docs/README.md). Key groups:

- [`docs/00-overview/`](docs/00-overview/README.md) — architecture, principles, capability/config/dependency.
- [`docs/01-kernel-core/`](docs/01-kernel-core/README.md) — task, scheduler, context switch, interrupt, timeout, invariants.
- [`docs/02-synchronization/`](docs/02-synchronization/README.md) — queue/semaphore/mutex/timer/suspend-resume.
- [`docs/03-haievent/`](docs/03-haievent/README.md) — event ownership, AO, FSM, time event, pub-sub.
- [`docs/04-platform/`](docs/04-platform/README.md) — Cortex-M3/STM32/target/porting.
- [`docs/05-api-reference/`](docs/05-api-reference/README.md) — public API contract.
- [`docs/06-testing-and-quality/`](docs/06-testing-and-quality/README.md) — tests, diagnostics, benchmark, release.
- [`docs/07-labs-and-examples/`](docs/07-labs-and-examples/README.md) — learning map + allocator lab.
- [`docs/08-appendices/`](docs/08-appendices/README.md) — glossary/source map/limitations.
- [`docs/09-version2/`](docs/09-version2/README.md) — **future plan only**.

<a id="limits"></a>
## v1 Limitations

- single-core only;
- no FPU context management or MPU isolation;
- Cortex-M3 critical sections use PRIMASK; there is no BASEPRI-based application interrupt ceiling yet;
- no tickless idle;
- no general-purpose dynamic kernel heap;
- `haievent` provides only a flat FSM and one task per AO;
- the only complete target currently provided is the STM32F103C8T6 Blue Pill;
- benchmarks are target/build evidence, not a hard real-time certification.

<a id="references"></a>
## References

**Primary official references:**

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)
- [ST RM0008 — STM32F101/102/103/105/107 Reference Manual](https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
- [STM32F103 Documentation](https://www.st.com/en/microcontrollers-microprocessors/stm32f103/documentation.html)
- [CMake — Toolchain Files](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html)
- [OpenOCD User's Guide](https://openocd.org/doc/html/index.html)

**Implementation sources in the repository:**
- `config/hairtos_config.h`
- `kernel/src/hr_kernel.c`
- `kernel/src/hr_task.c`
- `kernel/src/hr_scheduler.c`
- `kernel/src/hr_timeout.c`
- `kernel/src/hr_wait.c`
- `arch/arm/cortex-m3/hr_port.c`
- `arch/arm/cortex-m3/hr_port_stack.c`
- `arch/arm/cortex-m3/hr_portasm.S`
- `config/haievent_config.h`
- `haievent/src/he_event.c`
- `haievent/src/he_active.c`
- `haievent/src/he_state_machine.c`
- `haievent/src/he_time_event.c`
- `haievent/src/he_pubsub.c`
- `soc/stm32f1/startup_stm32f103.S`
- `soc/stm32f1/system_stm32f1.c`
- `soc/stm32f1/stm32f1_clock.c`
- `boards/bluepill_f103c8/board.c`
- `boards/bluepill_f103c8/STM32F103C8Tx_FLASH.ld`
- `cmake/targets/bluepill_f103c8.cmake`
