# 00 — Project Overview and Analysis

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
| [`architecture.md`](architecture.md) | hairtos Architecture |
| [`capability-matrix.md`](capability-matrix.md) | hairtos 1.0.0-rc1 Capability Matrix |
| [`coding-standard.md`](coding-standard.md) | Coding standard |
| [`configuration.md`](configuration.md) | Compile-Time Configuration |
| [`dependency-rules.md`](dependency-rules.md) | Dependency Rules |
| [`design-principles.md`](design-principles.md) | Design Principles |
| [`project-analysis.md`](project-analysis.md) | Full hairtos v1 Project Analysis |
| [`project-layout.md`](project-layout.md) | Repository Layout and Responsibilities |
| [`roadmap.md`](roadmap.md) | Roadmap |

<a id="validation"></a>
## Validation baseline

- `VERSION`: `1.0.0-rc1`.
- Host validation baseline: all 64 test functions in the current suite pass.
- `02-kernel-data-structures-host`, `14-memory-allocator-lab`, and `16-diagnostics-stress-stabilization` pass on the host.
- The reference target is `bluepill_f103c8`; host evidence does not replace cross-build, OpenOCD, and on-board hardware validation.

<a id="references"></a>
## References

- [CMake — CMAKE_TOOLCHAIN_FILE](https://cmake.org/cmake/help/latest/variable/CMAKE_TOOLCHAIN_FILE.html)
- [CMake — CMAKE_EXPORT_COMPILE_COMMANDS](https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html)

**Implementation sources in the repository:**
- `README.md`
- `CMakeLists.txt`
- `cmake/hairtos_examples.cmake`
- `cmake/hairtos_targets.cmake`
