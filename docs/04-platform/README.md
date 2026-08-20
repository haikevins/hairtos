# 04 — Platform, Architecture Port, and Targets

> **Role:** This page is an index for a documentation group. Each chapter moves from core concept → source mapping → invariant → failure mode → validation so readers can compare the documentation directly with the implementation.

[← Root README](../../README.md)

## Table of Contents

- [Content Map](#content-map)
- [How to Read This Section](#reading-guide)
- [Documents](#documents)
- [Validation baseline](#validation)
- [References](#references)

<a id="content-map"></a>
## Content Map

```mermaid
flowchart LR
    INDEX["Index"] --> CONCEPT["Concept / contract"]
    CONCEPT --> SOURCE["Source mapping"]
    SOURCE --> TEST["Validation / evidence"]
```

<a id="reading-guide"></a>
## How to Read This Section

1. Start with the section README to understand the scope and recommended learning order.
2. When an API appears, refer to `docs/05-api-reference/` for context and return-value contracts; for kernel behavior, prioritize `docs/01`–`03`.
3. Cross-check every timing and ownership statement against the source map at the end of the chapter.
4. Clearly distinguish **host evidence**, **target evidence**, and **future proposals**.

<a id="documents"></a>
## Documents

| Document | Role |
| --- | --- |
| [`cortex-m3-port.md`](cortex-m3-port.md) | ARM Cortex-M3 port |
| [`drivers.md`](drivers.md) | Driver abstraction |
| [`port-contract.md`](port-contract.md) | Architecture port contract |
| [`porting-guide.md`](porting-guide.md) | Porting Guide — Design Approach |
| [`porting-new-target.md`](porting-new-target.md) | Porting a New MCU Target |
| [`startup-and-linker.md`](startup-and-linker.md) | Startup and Linker Contract |
| [`stm32f103-platform.md`](stm32f103-platform.md) | STM32F103 / Blue Pill target |
| [`target-manifest.md`](target-manifest.md) | Target manifest |

<a id="validation"></a>
## Validation baseline

- `VERSION`: `1.0.0-rc1`.
- Host validation baseline: all 64 test functions in the current suite pass.
- `02-kernel-data-structures-host`, `14-memory-allocator-lab`, and `16-diagnostics-stress-stabilization` pass on the host.
- The reference target is `bluepill_f103c8`; host evidence does not replace cross-build, OpenOCD, and on-board hardware validation.

<a id="references"></a>
## References

- [ST RM0008 — STM32F10x Reference Manual](https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
- [ST PM0056 — STM32F10xxx Cortex-M3 Programming Manual](https://www.st.com/resource/en/programming_manual/cd00228163-stm32f10xxx20xxx21xxxl1xxxx-cortexm3-programming-manual-stmicroelectronics.pdf)
- [STM32F103 documentation portal](https://www.st.com/en/microcontrollers-microprocessors/stm32f103/documentation.html)

**Implementation sources in the repository:**
- `README.md`
- `CMakeLists.txt`
- `cmake/hairtos_examples.cmake`
- `cmake/hairtos_targets.cmake`
